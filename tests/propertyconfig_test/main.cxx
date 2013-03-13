#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/loggingmacros.h>


using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;



int
main()
{
    tcout << LOG4CPLUS_TEXT("Entering main()...") << endl;
    log4cplus::initialize ();
    LogLog::getLogLog()->setInternalDebugging(true);
    Logger root = Logger::getRoot();
    try {
        PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT("log4cplus.properties"));
        Logger fileCat = Logger::getInstance(LOG4CPLUS_TEXT("filelogger"));

	LOG4CPLUS_WARN(root, LOG4CPLUS_TEXT("Testing...."));

	LOG4CPLUS_WARN(root, LOG4CPLUS_TEXT("Writing messages to log...."));
        for(int i=0; i<10000; ++i) {
	    LOG4CPLUS_WARN(fileCat, LOG4CPLUS_TEXT("This is a WARNING...") << i);
        }
    }
    catch(...) {
        tcout << LOG4CPLUS_TEXT("Exception...") << endl;
	LOG4CPLUS_FATAL(root, LOG4CPLUS_TEXT("Exception occured..."));
    }

    tcout << LOG4CPLUS_TEXT("Exiting main()...") << endl;
    return 0;
}

