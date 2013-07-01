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
        PropertyConfigurator::doConfigure(
            LOG4CPLUS_TEXT("log4cplus.properties"));
        Logger fileCat = Logger::getInstance(LOG4CPLUS_TEXT("filelogger"));

	LOG4CPLUS_WARN(root, LOG4CPLUS_TEXT("Testing...."));
	LOG4CPLUS_WARN(root, LOG4CPLUS_TEXT("Writing messages to log...."));
        for (int i=0; i<10000; ++i)
	    LOG4CPLUS_WARN(fileCat, LOG4CPLUS_TEXT("This is a WARNING...")
                << i);

        // Test that DOS EOLs in property files get removed.
#define TEST_TEXT LOG4CPLUS_TEXT ("this is a test with DOS EOL-->")
        tistringstream propsStream (
            LOG4CPLUS_TEXT ("text=") TEST_TEXT LOG4CPLUS_TEXT ("\r\n"));
        Properties props (propsStream);
        LOG4CPLUS_ASSERT (root,
            props.getProperty (LOG4CPLUS_TEXT ("text")) == TEST_TEXT);
    }
    catch(...) {
        tcout << LOG4CPLUS_TEXT("Exception...") << endl;
	LOG4CPLUS_FATAL(root, LOG4CPLUS_TEXT("Exception occured..."));
    }

    tcout << LOG4CPLUS_TEXT("Exiting main()...") << endl;
    return 0;
}

