
#include <log4cplus/category.h>
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
    helpers::getLogLog().setInternalDebugging(true);
    SharedAppenderPtr append_1(new RollingFileAppender("Test.log", 5*1024, 5));
    append_1->setName("First");
    append_1->setLayout( std::auto_ptr<Layout>(new TTCCLayout()) );
    Category::getRoot().addAppender(append_1);

    Category root = Category::getRoot();
    Category test = Category::getInstance("test");
    Category subTest = Category::getInstance("test.subtest");

    for(int i=0; i<LOOP_COUNT; ++i) {
        NDCContextCreator _context("loop");
        LOG4CPLUS_DEBUG(subTest, "Entering loop #" << i)
    }


    return 0;
}


