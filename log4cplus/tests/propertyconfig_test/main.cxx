
#include <log4cplus/logger.h>
#include <log4cplus/propertyconfig.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::thread;



int
main()
{
    cout << "Entering main()..." << endl;
    getLogLog().setInternalDebugging(true);
    Logger root = Logger::getRoot();
    try {
        PropertyConfigurator configurator("log4cplus.properties");
        configurator.configure();
        Logger fileCat = Logger::getInstance("filelogger");

	LOG4CPLUS_WARN_STR(root, "Testing....")

	LOG4CPLUS_WARN_STR(root, "Writing messages to log....")
        for(int i=0; i<10000; ++i) {
            LOG4CPLUS_WARN(fileCat, "This is a WARNING..." << i)
        }
    }
    catch(...) {
        cout << "Exception..." << endl;
	LOG4CPLUS_FATAL_STR(root, "Exception occured...")
    }

    cout << "Exiting main()..." << endl;
    return 0;
}

