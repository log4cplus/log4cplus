#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/consoleappender.h>

using namespace std;
using namespace log4cplus;
using namespace log4cplus::spi;
using namespace log4cplus::helpers;


static Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("log"));

void
printDebug()
{
    LOG4CPLUS_TRACE_METHOD(logger, LOG4CPLUS_TEXT("::printDebug()"));
    LOG4CPLUS_DEBUG(logger, "This is a DEBUG message");
    LOG4CPLUS_INFO(logger, "This is a INFO message");
    LOG4CPLUS_WARN(logger, "This is a WARN message");
    LOG4CPLUS_ERROR(logger, "This is a ERROR message");
    LOG4CPLUS_FATAL(logger, "This is a FATAL message");

    LOG4CPLUS_INFO(logger, "visible");
    LOG4CPLUS_INFO(logger, "invisible");
}


void
test_1 (Logger & root)
{
    PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT("log4cplus.properties"));
    LOG4CPLUS_WARN(root, "Testing....");
}


void
test_2 (Logger & root)
{
    LOG4CPLUS_WARN(root, "Testing std::function<> filter....");
    root.removeAllAppenders ();
    auto filterFunction = [=](const InternalLoggingEvent& event)
        -> FilterResult {
        log4cplus::tstring const & msg = event.getMessage ();
        if (event.getLogLevel () >= WARN_LOG_LEVEL)
        {
            getLogLog ().debug (
                LOG4CPLUS_TEXT ("function filter: going neutral on event: ")
                + msg);

            return NEUTRAL;
        }
        else if (msg == LOG4CPLUS_TEXT ("visible"))
        {
            getLogLog ().debug (
                LOG4CPLUS_TEXT ("function filter: accepting event: ")
                + msg);
            return ACCEPT;
        }
        else
        {
            getLogLog ().debug (
                LOG4CPLUS_TEXT ("function filter: denying event: ")
                + msg);
            return DENY;
        }
    };

    SharedAppenderPtr appender (new ConsoleAppender);
    appender->setName (LOG4CPLUS_TEXT ("SECOND"));
    appender->addFilter (FilterPtr (new FunctionFilter (filterFunction)));
    root.addAppender (std::move (appender));
}


void
test_3 (Logger & root)
{
    LOG4CPLUS_WARN(root,
        LOG4CPLUS_TEXT("Testing std::function<> filter,")
        LOG4CPLUS_TEXT(" 2nd overload of addFilter()...."));
    root.removeAllAppenders ();
    SharedAppenderPtr appender (new ConsoleAppender);
    appender->setName (LOG4CPLUS_TEXT ("THIRD"));
    appender->addFilter (
        [=](const InternalLoggingEvent & event)
        -> FilterResult {
            getLogLog ().debug (
                LOG4CPLUS_TEXT ("function filter: accepting event: ")
                + event.getMessage ());
            return ACCEPT;
        });
    root.addAppender (std::move (appender));
}


int
main()
{
    tcout << "Entering main()..." << endl;
    log4cplus::initialize ();
    LogLog::getLogLog()->setInternalDebugging(true);
    Logger root = Logger::getRoot();
    try {
        // Test filters set up through properties file.

        test_1 (root);
        printDebug();

        // Test custom function filter.

        test_2 (root);
        printDebug();

        // Test custom function filter added using second overload of
        // `addFilter()`.

        test_3 (root);
        printDebug();
    }
    catch(...) {
        tcout << "Exception..." << endl;
        LOG4CPLUS_FATAL(root, "Exception occurred...");
    }

    tcout << "Exiting main()..." << endl;
    return 0;
}
