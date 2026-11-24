// SPDX-License-Identifier: Apache-2.0
#include <log4cplus/config.hxx>

#ifdef LOG4CPLUS_HAVE_OS_LOG
#include <log4cplus/oslogappender.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/spi/loggingevent.h>
#include "posixutf8.h"

#include <os/log.h>
#include <stdexcept>
#include <utility>

#ifdef LOG4CPLUS_WITH_UNIT_TESTS
#include <catch_amalgamated.hpp>
#endif

namespace log4cplus
{
namespace
{

os_log_type_t
logType (LogLevel level) noexcept
{
    if (level < INFO_LOG_LEVEL)
        return OS_LOG_TYPE_DEBUG;
    if (level < WARN_LOG_LEVEL)
        return OS_LOG_TYPE_INFO;
    if (level < ERROR_LOG_LEVEL)
        return OS_LOG_TYPE_DEFAULT;
    if (level < FATAL_LOG_LEVEL)
        return OS_LOG_TYPE_ERROR;
    return OS_LOG_TYPE_FAULT;
}

bool
publicMessage (helpers::Properties const & props)
{
    auto const privacy = props.getProperty (LOG4CPLUS_TEXT ("Privacy"), LOG4CPLUS_TEXT ("private"));
    if (privacy == LOG4CPLUS_TEXT ("private"))
        return false;
    if (privacy == LOG4CPLUS_TEXT ("public"))
        return true;
    throw std::invalid_argument ("OSLogAppender Privacy must be private or public");
}

} // namespace

struct OSLogAppender::Log
{
    internal::PosixUtf8Encoder encoder;
    bool const isPublic;
    os_log_t handle;

    explicit Log (helpers::Properties const & props)
        : isPublic (publicMessage (props))
    {
        auto subsystem = encoder.utf8 (props.getProperty (
            LOG4CPLUS_TEXT ("Subsystem"), LOG4CPLUS_TEXT ("org.log4cplus")));
        auto category = encoder.utf8 (props.getProperty (
            LOG4CPLUS_TEXT ("Category"), LOG4CPLUS_TEXT ("default")));
        handle = os_log_create (subsystem.c_str (), category.c_str ());
    }

    ~Log () { os_release (handle); }
    Log (Log const &) = delete;
    Log & operator= (Log const &) = delete;
};

OSLogAppender::OSLogAppender ()
    : OSLogAppender (helpers::Properties ())
{ }

OSLogAppender::OSLogAppender (helpers::Properties const & props)
    : OSLogAppender (props, std::make_unique<Log> (props))
{ }

OSLogAppender::OSLogAppender (helpers::Properties const & props, std::unique_ptr<Log> nativeLog)
    : Appender (props)
    , log (std::move (nativeLog))
{ }

OSLogAppender::~OSLogAppender ()
{
    destructorImpl ();
}

void
OSLogAppender::close ()
{
    thread::MutexGuard guard (access_mutex);
    closed = true;
    log.reset ();
}

void
OSLogAppender::append (spi::InternalLoggingEvent const & event)
{
    auto const type = logType (event.getLogLevel ());
    if (!os_log_type_enabled (log->handle, type))
        return;

    auto const & formatted = formatEvent (event);
    // A user-supplied layout can close this appender while formatting.
    if (closed)
        return;
    auto message = log->encoder.utf8 (formatted);
    // Preserve the rest of the layout when passing embedded NULs to %s.
    for (auto pos = message.find ('\0'); pos != message.npos; pos = message.find ('\0', pos + 2))
        message.replace (pos, 1, "\\0");
    if (log->isPublic)
        os_log_with_type (log->handle, type, "%{public}s", message.c_str ());
    else
        os_log_with_type (log->handle, type, "%{private}s", message.c_str ());
}

#ifdef LOG4CPLUS_WITH_UNIT_TESTS
CATCH_TEST_CASE ("OSLogAppender severity thresholds", "[oslog][appender]")
{
    CATCH_CHECK (logType (TRACE_LOG_LEVEL) == OS_LOG_TYPE_DEBUG);
    CATCH_CHECK (logType (DEBUG_LOG_LEVEL) == OS_LOG_TYPE_DEBUG);
    CATCH_CHECK (logType (INFO_LOG_LEVEL) == OS_LOG_TYPE_INFO);
    CATCH_CHECK (logType (WARN_LOG_LEVEL - 1) == OS_LOG_TYPE_INFO);
    CATCH_CHECK (logType (WARN_LOG_LEVEL) == OS_LOG_TYPE_DEFAULT);
    CATCH_CHECK (logType (ERROR_LOG_LEVEL) == OS_LOG_TYPE_ERROR);
    CATCH_CHECK (logType (FATAL_LOG_LEVEL) == OS_LOG_TYPE_FAULT);
}

CATCH_TEST_CASE ("OSLogAppender message privacy", "[oslog][appender]")
{
    helpers::Properties props;
    CATCH_CHECK_FALSE (publicMessage (props));
    props.setProperty (LOG4CPLUS_TEXT ("Privacy"), LOG4CPLUS_TEXT ("public"));
    CATCH_CHECK (publicMessage (props));
    props.setProperty (LOG4CPLUS_TEXT ("Privacy"), LOG4CPLUS_TEXT ("off"));
    CATCH_CHECK_THROWS_AS (OSLogAppender {props}, std::invalid_argument);
}
#endif

} // namespace log4cplus
#endif // LOG4CPLUS_HAVE_OS_LOG
