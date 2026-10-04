#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <exception>
#include <stdexcept>
#include <string>
#include <thread>

namespace {

constexpr int event_count = 100;
constexpr DWORD termination_status = 2;

struct Fixture
{
    void * (*create_initializer) (int);
    void (*destroy_initializer) (void *);
    void (*configure) (int, int);
    int (*completed) ();
    void (*drain) ();
    int (*deinitialize) ();
};

Fixture fixture;

template <typename Function>
Function
load_function (HMODULE module, char const * name)
{
    auto function = GetProcAddress (module, name);
#if defined (_M_IX86)
    // MSVC decorates 32-bit cdecl exports with a leading underscore.
    if (! function)
    {
        const std::string decorated = "_" + std::string (name);
        function = GetProcAddress (module, decorated.c_str ());
    }
#endif
    if (! function)
        throw std::runtime_error (name);
    return reinterpret_cast<Function> (function);
}

void
cleanup_at_exit ()
{
    try
    {
        if (fixture.deinitialize () == event_count)
            return;
    }
    catch (...)
    { }
    TerminateProcess (GetCurrentProcess (), 90);
}

struct InitializerOwner
{
    void * initializer;

    ~InitializerOwner ()
    {
        fixture.destroy_initializer (initializer);
    }
};

int
run_child (wchar_t const * dll_path, std::wstring const & mode, bool shrink)
{
    HMODULE module = LoadLibraryW (dll_path);
    if (! module)
        throw std::runtime_error ("LoadLibraryW failed");

    fixture.create_initializer = load_function<decltype(fixture.create_initializer)>
        (module, "fixture_create_initializer");
    fixture.destroy_initializer = load_function<decltype(fixture.destroy_initializer)>
        (module, "fixture_destroy_initializer");
    fixture.configure = load_function<decltype(fixture.configure)>
        (module, "fixture_configure");
    fixture.completed = load_function<decltype(fixture.completed)>
        (module, "fixture_completed");
    fixture.drain = load_function<decltype(fixture.drain)>
        (module, "fixture_drain");
    fixture.deinitialize = load_function<decltype(fixture.deinitialize)>
        (module, "fixture_deinitialize");

    {
        const bool dll_deinitialize = mode == L"dll_deinitialize"
            || mode == L"dll_deinitialize_exitprocess" || mode == L"dll_unload";
        const bool dll_owned = dll_deinitialize || mode == L"dll_initializer"
            || mode == L"dll_initializer_exitprocess";
        // This automatic owner is deliberately left alive when the application
        // thread calls exit(), just like Initializer at the top of main in #408.
        // DLL ownership 1 exercises Initializer destruction; 2 additionally
        // calls deinitialize() directly before that destruction.
        InitializerOwner owner {fixture.create_initializer (
            dll_deinitialize ? 2 : (dll_owned ? 1 : 0))};
        const bool pending = mode == L"exit_pending"
            || mode == L"exitprocess_pending" || (dll_owned && mode != L"dll_unload");
        fixture.configure (shrink, pending);

        if (mode == L"exit")
            fixture.drain (); // Isolate the original pool shutdown failure.
        else if (mode == L"atexit")
        {
            // Register in the executable's CRT, before ExitProcess kills workers.
            if (std::atexit (cleanup_at_exit) != 0)
                throw std::runtime_error ("atexit registration failed");
        }
        else if (mode == L"explicit" || mode == L"unload" || mode == L"dll_unload")
        {
            if (fixture.deinitialize () != event_count)
                throw std::runtime_error ("explicit shutdown lost events");
        }

        if (mode != L"normal" && mode != L"unload" && mode != L"dll_unload")
        {
            std::thread exiting ([&] {
                if (mode == L"exitprocess_pending" || mode == L"dll_initializer_exitprocess"
                    || mode == L"dll_deinitialize_exitprocess")
                    ExitProcess (termination_status);
                std::exit (termination_status);
            });
            exiting.join ();
            throw std::runtime_error ("exit unexpectedly returned");
        }
    }

    if (fixture.completed () != event_count)
        throw std::runtime_error ("orderly shutdown lost events");
    if (mode == L"unload" || mode == L"dll_unload")
    {
        if (! FreeLibrary (module) || GetModuleHandleW (dll_path))
            throw std::runtime_error ("fixture did not unload");
    }
    return 0;
}

int
run_parent (wchar_t const * dll_path, std::wstring const & mode, bool shrink)
{
    wchar_t executable[32768];
    DWORD length = GetModuleFileNameW (nullptr, executable,
        sizeof(executable) / sizeof(executable[0]));
    if (! length || length == sizeof(executable) / sizeof(executable[0]))
        throw std::runtime_error ("cannot obtain executable path");
    std::wstring command = L"\"" + std::wstring (executable) + L"\" --child \""
        + dll_path + L"\" " + mode + (shrink ? L" 1" : L" 0");
    STARTUPINFOW startup {};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process {};
    if (! CreateProcessW (executable, &command[0], nullptr, nullptr, FALSE,
            0, nullptr, nullptr, &startup, &process))
        throw std::runtime_error ("CreateProcessW failed");
    CloseHandle (process.hThread);

    const DWORD wait_result = WaitForSingleObject (process.hProcess, 15000);
    if (wait_result != WAIT_OBJECT_0)
    {
        TerminateProcess (process.hProcess, 91);
        WaitForSingleObject (process.hProcess, 1000);
        CloseHandle (process.hProcess);
        throw std::runtime_error ("child did not finish within 15 seconds");
    }
    DWORD status = 0;
    const BOOL got_status = GetExitCodeProcess (process.hProcess, &status);
    CloseHandle (process.hProcess);
    const DWORD expected = mode == L"normal" || mode == L"unload" || mode == L"dll_unload"
        ? 0 : termination_status;
    if (! got_status || status != expected)
    {
        std::fprintf (stderr, "child exit status %lu, expected %lu\n",
            static_cast<unsigned long> (status), static_cast<unsigned long> (expected));
        return 1;
    }
    return 0;
}

} // namespace

int
wmain (int argc, wchar_t ** argv)
{
    SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    std::set_terminate ([] {
        TerminateProcess (GetCurrentProcess (), 86);
        std::_Exit (86);
    });
    try
    {
        if (argc != 5 || (std::wcscmp (argv[1], L"--parent") != 0
                && std::wcscmp (argv[1], L"--child") != 0))
            throw std::runtime_error ("expected --parent/--child DLL MODE SHRINK");
        const std::wstring mode (argv[3]);
        if (mode != L"exit" && mode != L"exit_pending" && mode != L"exitprocess_pending"
            && mode != L"dll_initializer" && mode != L"dll_initializer_exitprocess"
            && mode != L"dll_deinitialize" && mode != L"dll_deinitialize_exitprocess"
            && mode != L"normal" && mode != L"explicit"
            && mode != L"atexit" && mode != L"unload" && mode != L"dll_unload")
            throw std::runtime_error ("unknown shutdown mode");
        if (std::wcscmp (argv[4], L"0") != 0 && std::wcscmp (argv[4], L"1") != 0)
            throw std::runtime_error ("SHRINK must be 0 or 1");
        const bool shrink = std::wcscmp (argv[4], L"1") == 0;
        if (std::wcscmp (argv[1], L"--parent") == 0)
            return run_parent (argv[2], mode, shrink);
        return run_child (argv[2], mode, shrink);
    }
    catch (std::exception const & error)
    {
        std::fprintf (stderr, "%s\n", error.what ());
        return 1;
    }
}
