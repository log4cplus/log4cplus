
#include <log4cplus/category.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/ndc.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/threads.h>
#include <iostream>
#include <string>

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::thread;

int
main()
{
    cout << "Entering main()..." << endl;
    getLogLog().setInternalDebugging(true);
    try {
        SharedObjectPtr<Appender> append_1(new ConsoleAppender());
        append_1->setName("First");

//	std::string pattern = "%d{%m/%d/%y %H:%M:%S} [%t] %-5p %c{2} %%%x%% - %m [%l]%n";
	std::string pattern = "%d{%c} [%t] %-5p [%.15c{3}] %%%x%% - %m [%l]%n";
	append_1->setLayout( std::auto_ptr<Layout>(new PatternLayout(pattern)) );
        Category::getRoot().addAppender(append_1);

        Category cat = Category::getInstance("test.a.long_logger_name.c.logger");
        LOG4CPLUS_DEBUG(cat, "This is the FIRST log message...")

	sleep(1, 0);
	{
            NDCContextCreator ndc("second");
            LOG4CPLUS_INFO(cat, "This is the SECOND log message...")
	}

	sleep(1, 0);
        LOG4CPLUS_WARN(cat, "This is the THIRD log message...")

	sleep(1, 0);
        LOG4CPLUS_ERROR(cat, "This is the FOURTH log message...")

	sleep(1, 0);
        LOG4CPLUS_FATAL(cat, "This is the FOURTH log message...")
    }
    catch(...) {
        cout << "Exception..." << endl;
        Category::getRoot().fatal("Exception occured...");
    }

    cout << "Exiting main()..." << endl;
    return 0;
}

