// SPDX-License-Identifier: Apache-2.0
#ifndef LOG4CPLUS_OS_LOG_APPENDER_HEADER_
#define LOG4CPLUS_OS_LOG_APPENDER_HEADER_

#include <log4cplus/config.hxx>

#ifdef LOG4CPLUS_HAVE_OS_LOG
#include <log4cplus/appender.h>
#include <memory>

namespace log4cplus
{

/** Sends layout-formatted events to macOS Unified Logging.
 * Subsystem and Category default to org.log4cplus and default.
 * Privacy accepts private (the default) or public for the entire message.
 */
class LOG4CPLUS_EXPORT OSLogAppender : public Appender
{
public:
    OSLogAppender ();
    explicit OSLogAppender (helpers::Properties const &);
    ~OSLogAppender () override;
    void close () override;

protected:
    void append (spi::InternalLoggingEvent const &) override;

private:
    struct LOG4CPLUS_PRIVATE Log;
    LOG4CPLUS_PRIVATE OSLogAppender (helpers::Properties const &, std::unique_ptr<Log>);
    std::unique_ptr<Log> log;
};

} // namespace log4cplus
#endif // LOG4CPLUS_HAVE_OS_LOG
#endif
