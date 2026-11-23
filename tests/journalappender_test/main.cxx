// SPDX-License-Identifier: Apache-2.0
#include <log4cplus/configurator.h>
#include <log4cplus/initializer.h>
#include <log4cplus/journalappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/mdc.h>
#include <log4cplus/spi/loggingevent.h>

#include <cerrno>
#include <clocale>
#include <iostream>
#include <map>
#include <stdexcept>
#include <sys/uio.h>

using namespace log4cplus;

namespace
{
std::map<std::string, std::string> fields;
int sendResult;
unsigned sends;

void check (bool condition, std::string const & what)
{
    if (!condition)
        throw std::runtime_error (what);
}

struct RecordingErrorHandler : ErrorHandler
{
    tstring message;
    void error (tstring const & text) override { message = text; }
    void reset () override { message.clear (); }
};
}

// Capture the public submission boundary, including iovec lengths. Exporting
// this symbol from the test executable also exercises shared-library builds.
extern "C" __attribute__ ((visibility ("default"))) int
sd_journal_sendv (iovec const * iov, int n)
{
    ++sends;
    fields.clear ();
    for (int i = 0; i < n; ++i)
    {
        std::string field (static_cast<char const *> (iov[i].iov_base), iov[i].iov_len);
        auto eq = field.find ('=');
        if (eq == field.npos || !fields.emplace (field.substr (0, eq), field.substr (eq + 1)).second)
            return -EINVAL;
    }
    return sendResult;
}

namespace
{
void checkMappings ()
{
    auto reject = [] (tstring const & key, tstring const & field) {
        helpers::Properties props;
        props.setProperty (LOG4CPLUS_TEXT ("MDCField.first"), LOG4CPLUS_TEXT ("DUP"));
        props.setProperty (LOG4CPLUS_TEXT ("MDCField.") + key, field);
        try
        {
            JournalAppender invalid (props);
        }
        catch (std::invalid_argument const &)
        {
            return;
        }
        throw std::runtime_error ("accepted invalid mapping");
    };
    for (auto dest : {LOG4CPLUS_TEXT ("_PID"), LOG4CPLUS_TEXT ("0KEY"),
             LOG4CPLUS_TEXT ("lower"), LOG4CPLUS_TEXT ("A.B"), LOG4CPLUS_TEXT ("MESSAGE"),
             LOG4CPLUS_TEXT ("SYSLOG_IDENTIFIER"), LOG4CPLUS_TEXT ("DUP"), LOG4CPLUS_TEXT ("")})
        reject (LOG4CPLUS_TEXT ("second"), dest);
    reject (LOG4CPLUS_TEXT ("second"), tstring (65, 'A'));
    reject (tstring (), LOG4CPLUS_TEXT ("EMPTY_KEY"));
}

void checkEvent ()
{
    helpers::Properties props;
    props.setProperty (LOG4CPLUS_TEXT ("log4cplus.rootLogger"), LOG4CPLUS_TEXT ("TRACE, J"));
    tstring prefix = LOG4CPLUS_TEXT ("log4cplus.appender.J");
    props.setProperty (prefix, LOG4CPLUS_TEXT ("log4cplus::JournalAppender"));
    props.setProperty (prefix + LOG4CPLUS_TEXT (".AsyncAppend"), LOG4CPLUS_TEXT ("true"));
    props.setProperty (prefix + LOG4CPLUS_TEXT (".Identifier"), LOG4CPLUS_TEXT ("billing"));
    props.setProperty (prefix + LOG4CPLUS_TEXT (".layout"), LOG4CPLUS_TEXT ("log4cplus::PatternLayout"));
    props.setProperty (prefix + LOG4CPLUS_TEXT (".layout.ConversionPattern"),
        LOG4CPLUS_TEXT ("%p [%X{request.id}] %m"));
    for (auto const & [key, dest] : MappedDiagnosticContextMap{
            {LOG4CPLUS_TEXT ("request.id"), LOG4CPLUS_TEXT ("REQUEST_ID")},
            {LOG4CPLUS_TEXT ("empty"), LOG4CPLUS_TEXT ("EMPTY")},
            {LOG4CPLUS_TEXT ("missing"), LOG4CPLUS_TEXT ("MISSING")}})
        props.setProperty (prefix + LOG4CPLUS_TEXT (".MDCField.") + key, dest);

    PropertyConfigurator (props).configure ();
    auto logger = Logger::getRoot ();
    auto appender = logger.getAppender (LOG4CPLUS_TEXT ("J"));
    check (appender.get () != nullptr, "factory registration");
    // The appender must keep decoding with the locale captured at construction.
    std::setlocale (LC_CTYPE, "C");
    getMDC ().put (LOG4CPLUS_TEXT ("request.id"), LOG4CPLUS_TEXT ("r42"));
    getMDC ().put (LOG4CPLUS_TEXT ("empty"), tstring ());
    getNDC ().push (LOG4CPLUS_TEXT ("context"));
    thread::setCurrentThreadName (LOG4CPLUS_TEXT ("producer"));
    thread::setCurrentThreadName2 (LOG4CPLUS_TEXT ("alternate"));
    auto message = tstring (LOG4CPLUS_TEXT ("a\0b\n  "), 6);
    std::string expected ("a\0b\n  ", 6);
#ifdef UNICODE
    message += L"\U0001f60a";
    message.push_back (static_cast<wchar_t> (0xd800));
    expected += "\xf0\x9f\x98\x8a\xef\xbf\xbd";
#else
    message += "\xf0\x9f\x98\x8a\xffx\xe2\x82";
    expected += "\xf0\x9f\x98\x8a\xef\xbf\xbdx\xef\xbf\xbd";
#endif
    message += LOG4CPLUS_TEXT ("  ");
    expected += "  ";
#ifndef UNICODE
    // An unfinished character at the end consumes the entire remaining prefix.
    message += "\xe2\x82";
    expected += "\xef\xbf\xbd";
#endif
    spi::InternalLoggingEvent event (logger.getName (), INFO_LOG_LEVEL, message, "pay.cxx", 42, "pay");
    logger.log (event);
    getMDC ().clear ();
    getNDC ().clear ();
    appender->waitToFinishAsyncLogging ();
    check (sends == 1, "one send per event");
    decltype (fields) const wanted {
            {"MESSAGE", "INFO [r42] " + expected}, {"LOG4CPLUS_MESSAGE", expected},
            {"PRIORITY", "6"}, {"LOG4CPLUS_LEVEL", "INFO"}, {"LOG4CPLUS_LEVEL_VALUE", "20000"},
            {"LOG4CPLUS_LOGGER", LOG4CPLUS_TSTRING_TO_STRING (logger.getName ())},
            {"LOG4CPLUS_TIMESTAMP_USEC", std::to_string (event.getTimestamp ().time_since_epoch ().count ())},
            {"CODE_FILE", "pay.cxx"}, {"CODE_LINE", "42"}, {"CODE_FUNC", "pay"},
            {"LOG4CPLUS_THREAD", "producer"}, {"LOG4CPLUS_THREAD2", "alternate"},
            {"LOG4CPLUS_NDC", "context"}, {"REQUEST_ID", "r42"}, {"EMPTY", ""},
            {"SYSLOG_IDENTIFIER", "billing"}};
    for (auto const & [field, value] : wanted)
        check (fields.at (field) == value, "unexpected value for " + field + ": " + fields.at (field));
    check (fields.size () == wanted.size (), "unexpected journal fields");
}

void checkLayoutsAndErrors ()
{
    JournalAppender appender;
    spi::InternalLoggingEvent event (LOG4CPLUS_TEXT ("logger"), WARN_LOG_LEVEL,
        LOG4CPLUS_TEXT ("message"), nullptr, -1);
    appender.doAppend (event);
    check (fields.at ("MESSAGE") == "WARN - message\n", "default layout");
    check (!fields.count ("CODE_FILE") && !fields.count ("CODE_LINE")
        && !fields.count ("CODE_FUNC") && !fields.count ("SYSLOG_IDENTIFIER"), "optional fields");

    auto original = fields;
    appender.setLayout (std::make_unique<PatternLayout> (LOG4CPLUS_TEXT ("%m")));
    appender.doAppend (event);
    original["MESSAGE"] = "message";
    check (fields == original, "setLayout changes only MESSAGE");

    for (auto [level, priority] : std::initializer_list<std::pair<LogLevel, char const *>>{
            {TRACE_LOG_LEVEL, "7"}, {DEBUG_LOG_LEVEL, "7"}, {INFO_LOG_LEVEL + 1, "6"},
            {WARN_LOG_LEVEL, "4"}, {ERROR_LOG_LEVEL, "3"}, {FATAL_LOG_LEVEL, "2"},
            {FATAL_LOG_LEVEL + 1, "1"}})
    {
        appender.doAppend (spi::InternalLoggingEvent (LOG4CPLUS_TEXT ("test"), level,
            LOG4CPLUS_TEXT ("m"), nullptr, -1));
        check (fields.at ("PRIORITY") == priority, "syslog priority");
    }
    auto handler = std::make_unique<RecordingErrorHandler> ();
    auto * record = handler.get ();
    appender.setErrorHandler (std::move (handler));
    sendResult = -EACCES;
    appender.doAppend (event);
    check (!record->message.empty (), "send error reported");
}
}

int main (int argc, char ** argv)
try
{
    Initializer initializer;
    if (argc == 2)
    {
        if (!std::setlocale (LC_CTYPE, argv[1]))
            return 77;
        JournalAppender appender;
        std::setlocale (LC_CTYPE, "C");
        appender.doAppend (spi::InternalLoggingEvent (LOG4CPLUS_TEXT ("latin1"), INFO_LOG_LEVEL,
            LOG4CPLUS_TEXT ("\xe9"), nullptr, -1));
        check (fields.at ("LOG4CPLUS_MESSAGE") == "\xc3\xa9", "Latin-1 to UTF-8");
        return 0;
    }
    check (std::setlocale (LC_CTYPE, "C.UTF-8") || std::setlocale (LC_CTYPE, "en_US.UTF-8"),
        "test requires a UTF-8 locale");
    checkMappings ();
    checkEvent ();
    checkLayoutsAndErrors ();
    return 0;
}
catch (std::exception const & e)
{
    std::cerr << e.what () << '\n';
    return 1;
}
