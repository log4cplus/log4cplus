# Windows DLL shutdown regressions

These tests apply to shared, multithreaded Windows builds with the internal
thread pool enabled. They reproduce log4cplus issue #408 without adding hooks
to the production library. The executable dynamically loads a consumer fixture
DLL, which imports the production log4cplus DLL:

```text
windows_shutdown_test.exe -> windows_shutdown_fixture.dll -> log4cplus.dll
```

The fixture's static initializer owner is constructed through an exported
function outside the loader lock. During process termination, its destruction
calls into log4cplus before log4cplus receives its own process-detach notification.
Compiling the library sources into the fixture would hide this ordering problem.

Every case runs in a subprocess. The parent waits at most 15 seconds and checks
the exit status; CTest adds a 25-second timeout. All checks remain active under
`NDEBUG`. Pending-event cases use a Windows event to confirm a worker has entered
its appender before termination. That worker remains blocked with the appender
lock held, making accidental cleanup observable as a hang.

The twelve cases run both with and without prior downsizing (24 tests):

- `exit`: an application thread calls `exit(2)` while its caller still owns an
  automatic initializer; logging has drained, isolating pool shutdown.
- `exit_pending` and `exitprocess_pending`: terminate with an asynchronous event
  still in flight and its appender lock held.
- `dll_initializer` and `dll_deinitialize`: terminate with a pending event and a
  DLL-owned initializer. The first exercises its destructor directly; the second
  additionally calls `deinitialize()` before destroying the initializer.
- `dll_initializer_exitprocess` and `dll_deinitialize_exitprocess`: exercise
  those same DLL teardown paths using direct `ExitProcess`.
- `normal`, `explicit`, and `atexit`: complete all 100 queued events through
  automatic destruction, explicit deinitialization, or an executable's exit hook.
- `unload`: explicitly deinitialize, destroy the automatic initializer, then
  call `FreeLibrary` and verify the fixture DLL has unloaded.
- `dll_unload`: explicitly deinitialize and verify all events completed, then
  unload the fixture while its DLL-owned initializer is still alive. Its
  destructor and explicit teardown cleanup must complete during ordinary unload.

## Building and running

On Windows, configure with `BUILD_SHARED_LIBS=ON`, `LOG4CPLUS_BUILD_TESTING=ON`,
and `LOG4CPLUS_ENABLE_THREAD_POOL=ON`, then build `windows_shutdown_test` and run:

```sh
ctest --test-dir build -C Debug -R '^windows_shutdown_' --output-on-failure
```

For Linux cross builds with MinGW and Wine:

```sh
cmake -S . -B build-windows -G Ninja \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc-posix \
  -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++-posix \
  -DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres \
  -DCMAKE_CROSSCOMPILING_EMULATOR=wine64 \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=ON \
  -DLOG4CPLUS_BUILD_TESTING=ON -DWITH_UNIT_TESTS=OFF \
  -DLOG4CPLUS_BUILD_LOGGINGSERVER=OFF \
  -DLOG4CPLUS_REQUIRE_EXPLICIT_INITIALIZATION=ON
cmake --build build-windows --target windows_shutdown_test -j4
ctest --test-dir build-windows -R '^windows_shutdown_' --output-on-failure
```

Make matching MinGW runtime DLLs available beside the executables or through
Wine's DLL search path. Repeat with the `-win32` compilers, with `Release`, and
with both the pinned ThreadPool header and the fixed PR header. An alternate
header can be supplied with `-DCMAKE_CXX_FLAGS=-I/absolute/header-directory`
without changing the submodule revision. Also check 32-bit linking using the
`i686-w64-mingw32` toolchain: the imported function uses the `NTAPI` convention.

MinGW GCC 13 can fail the production DLL's Release LTO link with duplicate
exported `LogLevelTranslator` vtables. For shutdown validation, keep `-O3 -DNDEBUG`
but disable the library's Release IPO with a temporary CMake project include:

```cmake
# validation-ipo.cmake
cmake_language(DEFER CALL set_property TARGET ${log4cplus}
  PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE OFF)
```

Pass `-DCMAKE_PROJECT_log4cplus_INCLUDE=/absolute/validation-ipo.cmake` when
configuring. This validation override does not require a production build change.

For an Autotools link check with GCC 13, use `CXXFLAGS='-std=gnu++23 -O0 -g'`.
That compiler reports the draft `__cplusplus` value `202100L`, so the existing
C++23 configure probe rejects it. A validation-only
`ax_cv_cxx_compile_cxx23=yes` environment override can bypass that probe while
still compiling and linking the complete library in C++23 mode. This does not
establish full C++23 conformance or change the production compiler requirement.

For native MSVC, run the following from a developer PowerShell with Ninja and
initialized submodules. This covers `/MD`, `/MDd`, `/MT`, and `/MTd` separately:

```powershell
foreach ($runtime in 'MultiThreadedDLL', 'MultiThreadedDebugDLL',
                     'MultiThreaded', 'MultiThreadedDebug') {
    $config = if ($runtime -match 'Debug') { 'Debug' } else { 'Release' }
    $build = "build-shutdown-$runtime"
    cmake -S . -B $build -G Ninja -DCMAKE_BUILD_TYPE=$config `
      -DCMAKE_MSVC_RUNTIME_LIBRARY=$runtime -DBUILD_SHARED_LIBS=ON `
      -DLOG4CPLUS_BUILD_TESTING=ON -DWITH_UNIT_TESTS=OFF `
      -DLOG4CPLUS_BUILD_LOGGINGSERVER=OFF `
      -DLOG4CPLUS_REQUIRE_EXPLICIT_INITIALIZATION=ON
    if ($LASTEXITCODE) { throw 'configure failed' }
    cmake --build $build --target windows_shutdown_test
    if ($LASTEXITCODE) { throw 'build failed' }
    ctest --test-dir $build -R '^windows_shutdown_' --output-on-failure
    if ($LASTEXITCODE) { throw 'tests failed' }
}
```

Verify with `dumpbin /imports` or MinGW `objdump -p` that the production DLL has
a normal import of `RtlDllShutdownInProgress` from `ntdll.dll`, rather than a
delay import. The fixture must import log4cplus, and the executable must not
import the fixture (so `FreeLibrary` can unload it).

## Boundaries

The DLL queries process-wide termination state before touching cleanup locks or
TLS. This also works when the DLL uses the static MSVC runtime and its own
`DllMain` is compiled out. During termination it abandons resources; pending
logging may be lost. Normal shutdown and explicit deinitialization still drain
logging, and dynamic unloading requires orderly shutdown beforehand.

Wine results do not establish native MSVC coverage. This change does not cover
static log4cplus embedded in another DLL, arbitrary logging during DLL teardown,
or concurrent external logging while orderly deinitialization is in progress.
