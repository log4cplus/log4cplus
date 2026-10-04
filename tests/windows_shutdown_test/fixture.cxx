#include <log4cplus/appender.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/initializer.h>
#include <log4cplus/logger.h>
#include <log4cplus/config/windowsh-inc-full.h>

#include <atomic>
#include <optional>
#include <stdexcept>

namespace {

constexpr int event_count = 100;
std::atomic<int> completed {0};
HANDLE entered = nullptr;
HANDLE release = nullptr;

// Constructed by an exported function outside the loader lock. Its destructor
// runs before the imported log4cplus DLL receives DLL_PROCESS_DETACH.
struct DllInitializer
{
    std::optional<log4cplus::Initializer> initializer;
    bool deinitialize_on_detach = false;

    ~DllInitializer ()
    {
        if (deinitialize_on_detach)
            log4cplus::deinitialize ();
    }
} dll_initializer;

class CountingAppender : public log4cplus::Appender
{
public:
    explicit CountingAppender (log4cplus::helpers::Properties const & properties)
        : Appender (properties)
    { }

    ~CountingAppender () override
    {
        destructorImpl ();
    }

    void close () override
    {
        closed = true;
    }

protected:
    void append (log4cplus::spi::InternalLoggingEvent const &) override
    {
        if (entered)
        {
            // Keep an asynchronous event in flight, and its appender lock held,
            // until Windows kills this worker during process termination.
            SetEvent (entered);
            if (WaitForSingleObject (release, INFINITE) != WAIT_OBJECT_0)
                throw std::runtime_error ("worker gate failed");
        }
        ++completed;
    }
};

} // namespace

extern "C" __declspec(dllexport) void *
fixture_create_initializer (int ownership)
{
    if (ownership)
    {
        dll_initializer.initializer.emplace ();
        dll_initializer.deinitialize_on_detach = ownership == 2;
        return nullptr;
    }
    return new log4cplus::Initializer;
}

extern "C" __declspec(dllexport) void
fixture_destroy_initializer (void * initializer)
{
    delete static_cast<log4cplus::Initializer *> (initializer);
}

extern "C" __declspec(dllexport) void
fixture_configure (int shrink, int pending)
{
    log4cplus::setThreadPoolSize (4);
    if (shrink)
        log4cplus::setThreadPoolSize (1);

    if (pending)
    {
        entered = CreateEventW (nullptr, TRUE, FALSE, nullptr);
        release = CreateEventW (nullptr, TRUE, FALSE, nullptr);
        if (! entered || ! release)
            throw std::runtime_error ("cannot create worker gates");
    }

    log4cplus::helpers::Properties properties;
    properties.setProperty (LOG4CPLUS_TEXT("AsyncAppend"),
        LOG4CPLUS_TEXT("true"));
    log4cplus::SharedAppenderPtr appender (new CountingAppender (properties));
    auto logger = log4cplus::Logger::getRoot ();
    logger.addAppender (appender);
    for (int i = 0; i != (pending ? 1 : event_count); ++i)
        logger.forcedLog (log4cplus::INFO_LOG_LEVEL,
            LOG4CPLUS_TEXT("queued event"));

    if (pending && WaitForSingleObject (entered, 5000) != WAIT_OBJECT_0)
        throw std::runtime_error ("asynchronous event did not start");
}

extern "C" __declspec(dllexport) int
fixture_completed ()
{
    return completed.load ();
}

extern "C" __declspec(dllexport) void
fixture_drain ()
{
    log4cplus::Logger::shutdown ();
}

extern "C" __declspec(dllexport) int
fixture_deinitialize ()
{
    log4cplus::deinitialize ();
    return completed.load ();
}
