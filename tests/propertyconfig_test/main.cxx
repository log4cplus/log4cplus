
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/streams.h>

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;



int
main()
{
    tcout << LOG4CPLUS_TEXT("Entering main()...") << endl;
    LogLog::getLogLog()->setInternalDebugging(true);
    Logger root = Logger::getRoot();
    try {
        PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT("log4cplus.properties"));
        Logger fileCat = Logger::getInstance(LOG4CPLUS_TEXT("filelogger"));

	LOG4CPLUS_WARN(root, "Testing....");

	LOG4CPLUS_WARN(root, "Writing messages to log....");
        for(int i=0; i<10000; ++i) {
            LOG4CPLUS_WARN(fileCat, "This is a WARNING..." << i);
        }
    }
    catch(...) {
        tcout << LOG4CPLUS_TEXT("Exception...") << endl;
	LOG4CPLUS_FATAL(root, "Exception occured...");
    }

    tcout << LOG4CPLUS_TEXT("Exiting main()...") << endl;
    return 0;
}

