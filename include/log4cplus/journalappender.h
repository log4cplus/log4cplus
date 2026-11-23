// SPDX-License-Identifier: Apache-2.0
#ifndef LOG4CPLUS_JOURNAL_APPENDER_HEADER_
#define LOG4CPLUS_JOURNAL_APPENDER_HEADER_

#include <log4cplus/config.hxx>

#if defined (LOG4CPLUS_WITH_SYSTEMD)
#include <log4cplus/appender.h>
#include <memory>

namespace log4cplus
{

/** Writes one structured systemd journal entry per event.
 *
 * The configured layout supplies MESSAGE. LOG4CPLUS_MESSAGE carries the
 * original text. Identifier overrides SYSLOG_IDENTIFIER. Each property
 * MDCField.<key>=FIELD exports the exact MDC key as a separate journal field.
 * Narrow strings use the LC_CTYPE locale active at construction. All text
 * is submitted as UTF-8, with malformed input replaced by U+FFFD.
 *
 * Submission can block. libsystemd silently accepts a missing journal socket.
 */
class LOG4CPLUS_EXPORT JournalAppender : public Appender
{
public:
    JournalAppender ();
    explicit JournalAppender (helpers::Properties const &);
    ~JournalAppender () override;
    void close () override;

protected:
    void append (spi::InternalLoggingEvent const &) override;

private:
    struct LOG4CPLUS_PRIVATE Options;
    LOG4CPLUS_PRIVATE JournalAppender (helpers::Properties const &, std::unique_ptr<Options const>);
    std::unique_ptr<Options const> options;
};

} // namespace log4cplus
#endif
#endif
