
#include <log4cplus/logger.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/ndc.h>
#include <log4cplus/helpers/loglog.h>
//#include <iomanip>

using namespace log4cplus;

#define LOOP_COUNT 2000


int
main()
{
    helpers::LogLog::getLogLog()->setInternalDebugging(true);
    SharedAppenderPtr append_1(new RollingFileAppender("Test.log", 5*1024, 5));
    append_1->setName("First");
    append_1->setLayout( std::auto_ptr<Layout>(new TTCCLayout()) );
    Logger::getRoot().addAppender(append_1);

    Logger root = Logger::getRoot();
    Logger test = Logger::getInstance("test");
    Logger subTest = Logger::getInstance("test.subtest");

    for(int i=0; i<LOOP_COUNT; ++i) {
        NDCContextCreator _context("loop");
        LOG4CPLUS_DEBUG(subTest, "Entering loop #" << i)
    }


    return 0;
}


