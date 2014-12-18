#include <exception>
#include <iostream>
#include <string>
#include <sstream>
#include "log4cplus/logger.h"
#include "log4cplus/consoleappender.h"
#include "log4cplus/helpers/appenderattachableimpl.h"
#include "log4cplus/helpers/loglog.h"
#include "log4cplus/helpers/pointer.h"
#include "log4cplus/helpers/property.h"
#include "log4cplus/spi/loggingevent.h"

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::spi;

static void
printAppenderList(const SharedAppenderPtrList& list)
{
   cout << "List size: " << list.size() << endl;
   for(SharedAppenderPtrList::const_iterator it=list.begin(); it!=list.end(); ++it) {
       log4cplus::tcout << "Loop Body: Appender name = " << (*it)->getName()
                        << endl;
   }
}


// This appender does not call destructorImpl(), which is wrong.
class BadDerivedAppender
    : public Appender
{
public:
    virtual void close ()
    { }

    virtual void append (const log4cplus::spi::InternalLoggingEvent&)
    { }
};


int
main()
{
    log4cplus::initialize ();
    LogLog::getLogLog()->setInternalDebugging(true);
    {
        try {
            SharedObjectPtr<Appender> append_1(
                new ConsoleAppender(false, true));
            append_1->setName(LOG4CPLUS_TEXT("First"));

            SharedObjectPtr<Appender> append_2(new ConsoleAppender());
            append_2->setName(LOG4CPLUS_TEXT("Second"));

            // Test that we get back the same layout as we set.

            Layout * layout_2;
            append_2->setLayout(
                std::auto_ptr<Layout>(layout_2 = new log4cplus::SimpleLayout));
            if (append_2->getLayout () != layout_2)
                return 1;

            // Default error handler should be set.

            if (! append_2->getErrorHandler ())
                return 2;

            // Test warning on NULL handler.

            append_2->setErrorHandler (std::auto_ptr<ErrorHandler>());

            // Set working error handler.

            std::auto_ptr<ErrorHandler> errorHandler (new OnlyOnceErrorHandler);
            append_2->setErrorHandler (errorHandler);

            // Test logging through instantiated appenders.

            InternalLoggingEvent event(
                Logger::getInstance(LOG4CPLUS_TEXT("test")).getName(),
                DEBUG_LOG_LEVEL, LOG4CPLUS_TEXT("This is a test..."), __FILE__,
                __LINE__, "main");

            AppenderAttachableImpl aai;
            aai.addAppender(append_1);
            aai.addAppender(append_2);
            aai.addAppender(append_1);
            aai.addAppender(append_2);
            aai.addAppender(SharedAppenderPtr());
            printAppenderList(aai.getAllAppenders());

            aai.removeAppender(append_2);
            printAppenderList(aai.getAllAppenders());

            aai.removeAppender(LOG4CPLUS_TEXT("First"));
            aai.removeAppender(SharedAppenderPtr());
            printAppenderList(aai.getAllAppenders());

            aai.addAppender(append_1);
            aai.addAppender(append_2);
            aai.addAppender(append_1);
            aai.addAppender(append_2);
            log4cplus::tcout << "Should be First: "
                             << aai.getAppender(LOG4CPLUS_TEXT("First"))->getName() << endl;
            log4cplus::tcout << "Should be Second: "
                             << aai.getAppender(LOG4CPLUS_TEXT("Second"))->getName() << endl
                             << endl;
            append_1->doAppend(event);
            append_2->doAppend(event);

            // Test appending to closed appender error handling.

            append_2->close ();
            append_2->doAppend (event);

            // Test appender's error handling for wrong layout.

            {
                tistringstream propsStream (
                    LOG4CPLUS_TEXT ("layout=log4cplus::WrongLayout"));
                Properties props (propsStream);
                SharedObjectPtr<Appender> append (
                    new ConsoleAppender (props));
                append->setName (LOG4CPLUS_TEXT ("Third"));
            }

            // Test threshold parsing.

            {
                tistringstream propsStream (
                    LOG4CPLUS_TEXT ("Threshold=ERROR"));
                Properties props (propsStream);
                SharedObjectPtr<Appender> append (
                    new ConsoleAppender (props));
                append->setName (LOG4CPLUS_TEXT ("Fourth"));
            }

            // Test threshold parsing of wrong log level.

            {
                tistringstream propsStream (
                    LOG4CPLUS_TEXT ("Threshold=WRONG"));
                Properties props (propsStream);
                SharedObjectPtr<Appender> append (
                    new ConsoleAppender (props));
                append->setName (LOG4CPLUS_TEXT ("Fifth"));
            }

            // Test wrong filter parsing.

            {
                tistringstream propsStream (
                    LOG4CPLUS_TEXT ("filters.1=log4cplus::spi::WrongFilter"));
                Properties props (propsStream);
                SharedObjectPtr<Appender> append (
                    new ConsoleAppender (props));
                append->setName (LOG4CPLUS_TEXT ("Sixth"));
            }

            // Test error reporting of badly coded appender.

            {
                BadDerivedAppender appender;
                appender.setName (LOG4CPLUS_TEXT ("Seventh"));
            }
        }
        catch(std::exception const & e) {
            log4cplus::tcout << "**** Exception occured: " << e.what() << endl;
        }
    }

    log4cplus::tcout << "Exiting main()..." << endl;
    return 0;
}
