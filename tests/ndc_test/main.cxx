
#include <log4cplus/logger.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/ndc.h>
#include <log4cplus/helpers/loglog.h>
#include <iostream>
#include <string>

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;

int
main()
{
    cout << "Entering main()..." << endl;
    LogLog::getLogLog()->setInternalDebugging(true);
    try {
        SharedObjectPtr<Appender> append_1(new ConsoleAppender());
        append_1->setName("First");
	append_1->setLayout( std::auto_ptr<Layout>(new log4cplus::TTCCLayout()) );
        Logger::getRoot().addAppender(append_1);

        Logger logger = Logger::getInstance("test");
        cout << "Logger: " << logger.getName() << endl;
        getNDC().push("tsmith");
        LOG4CPLUS_DEBUG(logger, "This is a short test...")

        getNDC().push("password");
        LOG4CPLUS_DEBUG(logger, "This should have my password now");

        getNDC().pop();
        LOG4CPLUS_DEBUG(logger, "This should NOT have my password now");

        getNDC().pop();
        cout << "Just returned from pop..." << endl;
        LOG4CPLUS_DEBUG(logger, "There should be no NDC...");

        getNDC().push("tsmith");
        getNDC().push("password");
        LOG4CPLUS_DEBUG(logger, "This should have my password now");
        getNDC().remove();
        LOG4CPLUS_DEBUG(logger, "There should be no NDC...");
    }
    catch(...) {
        cout << "Exception..." << endl;
        Logger::getRoot().log(FATAL_LOG_LEVEL, "Exception occured...");
    }

    cout << "Exiting main()..." << endl;
    return 0;
}

