
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
    std::string pattern = "%d{%m/%d/%y %H:%M:%S,%q} [%t] %-5p %c{2} %%%x%% - %m [%l]%n";
//  std::string pattern = "%d{%c} [%t] %-5p [%.15c{3}] %%%x%% - %m [%l]%n";
    std::auto_ptr<Layout> layout( new PatternLayout(pattern) );
//    std::auto_ptr<Layout> layout( new TTCCLayout() );
    append_1->setLayout(layout);
    Logger::getRoot().addAppender(append_1);

    Logger root = Logger::getRoot();
    Logger test = Logger::getInstance("test");
    Logger subTest = Logger::getInstance("test.subtest");

    for(int i=0; i<LOOP_COUNT; ++i) {
        NDCContextCreator _context("loop");
        LOG4CPLUS_DEBUG(subTest, "Entering loop #" << i);
    }


    return 0;
}


