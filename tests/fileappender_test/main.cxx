#include <log4cplus/logger.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/ndc.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/loggingmacros.h>


using namespace log4cplus;

const int LOOP_COUNT = 20000;


int
main()
{
    helpers::LogLog::getLogLog()->setInternalDebugging(true);
    SharedAppenderPtr append_1(
        new RollingFileAppender(LOG4CPLUS_TEXT("Test.log"), 5*1024, 5));
    append_1->setName(LOG4CPLUS_TEXT("First"));
    append_1->setLayout( std::auto_ptr<Layout>(new TTCCLayout()) );
    Logger::getRoot().addAppender(append_1);

    Logger root = Logger::getRoot();
    Logger test = Logger::getInstance(LOG4CPLUS_TEXT("test"));
    Logger subTest = Logger::getInstance(LOG4CPLUS_TEXT("test.subtest"));

    for(int i=0; i<LOOP_COUNT; ++i) {
        NDCContextCreator _context(LOG4CPLUS_TEXT("loop"));
        LOG4CPLUS_DEBUG(subTest, "Entering loop #" << i);
    }


    return 0;
}
