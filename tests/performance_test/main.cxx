
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;

log4cplus::tostream& operator <<(log4cplus::tostream& s, const Time& t)
{
    return s << t.sec() << "sec " << t.usec() << "usec";
}

#define LOOP_COUNT 100000

int
main()
{
    cout << "Entering main()..." << endl;
    PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT("log4cplus.properties"));
    Logger root = Logger::getRoot();
    try {
        Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("testlogger"));

        LOG4CPLUS_WARN(root, "Starting test loop....");
        Time start = Time::gettimeofday();
        tstring msg(LOG4CPLUS_TEXT("This is a WARNING..."));
        int i = 0;
        for(i=0; i<LOOP_COUNT; ++i) {
            LOG4CPLUS_WARN(logger, msg);
        }
        Time end = Time::gettimeofday();
        Time diff = end - start;
        LOG4CPLUS_WARN(root, "Logging " << LOOP_COUNT << " took: " << diff << endl);
        LOG4CPLUS_WARN(root, "Logging average: " << (diff/LOOP_COUNT) << endl);

        start = Time::gettimeofday();
        for(i=0; i<LOOP_COUNT; ++i) {
            tostringstream buffer;
	        buffer /*<< "test"*/ << 123122;
	        tstring tmp = buffer.str();
        }
        end = Time::gettimeofday();
        diff = end - start;
        LOG4CPLUS_WARN(root, "tostringstream average: " << (diff/LOOP_COUNT) << endl);

        start = Time::gettimeofday();
        for(i=0; i<LOOP_COUNT; ++i) {
	    log4cplus::spi::InternalLoggingEvent e(logger.getName(), log4cplus::WARN_LOG_LEVEL,
			                         msg, 0, 0);
        }
        end = Time::gettimeofday();
        diff = end - start;

        LOG4CPLUS_WARN(root, "Creating log " << LOOP_COUNT << " objects took: " << diff);
        LOG4CPLUS_WARN(root, "Creating log object average: " << (diff/LOOP_COUNT) << endl);

        start = Time::gettimeofday();
        for(i=0; i<LOOP_COUNT; ++i) {
	    log4cplus::spi::InternalLoggingEvent e(logger.getName(), log4cplus::WARN_LOG_LEVEL,
			                           msg, 0, 0);
	    e.getNDC();
	    e.getThread();
        }
        end = Time::gettimeofday();
        diff = end - start;
        LOG4CPLUS_WARN(root, "Creating FULL log " << LOOP_COUNT << " objects took: " << diff);
        LOG4CPLUS_WARN(root, "Creating FULL log object average: " << (diff/LOOP_COUNT) << endl);
        
        start = Time::gettimeofday();
        for(i=0; i<LOOP_COUNT; ++i) {
            log4cplus::spi::InternalLoggingEvent e(logger.getName(), log4cplus::WARN_LOG_LEVEL,
			                           msg, 0, 0);
	    e.getNDC();
        }
        end = Time::gettimeofday();
        diff = end - start;
        LOG4CPLUS_WARN(root, "getNDC() " << LOOP_COUNT << " calls took: " << diff);
        LOG4CPLUS_WARN(root, "getNDC() average: " << (diff/LOOP_COUNT) << endl);
        
        start = Time::gettimeofday();
        for(i=0; i<LOOP_COUNT; ++i) {
            log4cplus::spi::InternalLoggingEvent e(logger.getName(), log4cplus::WARN_LOG_LEVEL,
			                           msg, 0, 0);
	    e.getThread();
        }
        end = Time::gettimeofday();
        diff = end - start;
        LOG4CPLUS_WARN(root, "getThread() " << LOOP_COUNT << " calls took: " << diff);
        LOG4CPLUS_WARN(root, "getThread() average: " << (diff/LOOP_COUNT) << endl);
    }
    catch(...) {
        cout << "Exception..." << endl;
        LOG4CPLUS_FATAL(root, "Exception occured...");
    }

    cout << "Exiting main()..." << endl;
    return 0;
}


