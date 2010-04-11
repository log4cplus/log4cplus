
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>

using namespace std;
using namespace log4cplus;
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
}


int
main()
{
#ifdef WIN32
    cout << "=-=-=-= NOTICE =-=-=-=" << endl
         << "Make sure you copy the log4cplus\\tests\\filter_test\\log4cplus.properties"
         << " file into the directory you run this test from"
         << endl
         << "=-=-=-=-=-=-=-=-=-=-=-=" 
         << endl << endl;
#endif

    cout << "Entering main()..." << endl;
    LogLog::getLogLog()->setInternalDebugging(true);
    Logger root = Logger::getRoot();
    try {
        PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT("log4cplus.properties"));

        LOG4CPLUS_WARN(root, "Testing....");
        printDebug();
    }
    catch(...) {
        cout << "Exception..." << endl;
        LOG4CPLUS_FATAL(root, "Exception occured...");
    }

    cout << "Exiting main()..." << endl;
    return 0;
}

