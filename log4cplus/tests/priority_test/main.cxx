
#include "log4cplus/category.h"
#include "log4cplus/consoleappender.h"
#include "log4cplus/priority.h"
#include <iomanip>
#include <iostream>


using namespace std;
using namespace log4cplus;

// Forward Declarations
void writeLogMessage();


int
main()
{
    cout << "Entering main()..." << endl;
    {
    SharedAppenderPtr append_1(new ConsoleAppender());
    append_1->setName("First");
    cout << "Getting root category...DONE" << endl;
    Category::getRoot().addAppender(append_1);

    Category root = Category::getRoot();
    Category test = Category::getInstance("test");
    Category subTest = Category::getInstance("test.subtest");

    LOG4CPLUS_FATAL(root, "root: " << root.getChainedPriority())
    LOG4CPLUS_FATAL(root, "test: " << test.getChainedPriority())
    LOG4CPLUS_FATAL(root, "test.subtest: " << subTest.getChainedPriority())

    LOG4CPLUS_FATAL(root, "\nSetting test.subtest to WARN")
    subTest.setPriority(Priority::WARN_PRI);
    LOG4CPLUS_FATAL(root, "root: " << root.getChainedPriority())
    LOG4CPLUS_FATAL(root, "test: " << test.getChainedPriority())
    LOG4CPLUS_FATAL(root, "test.subtest: " << subTest.getChainedPriority())

    LOG4CPLUS_FATAL(root, "\nSetting test to ERROR")
    test.setPriority(Priority::ERROR_PRI);
    LOG4CPLUS_FATAL(root, "root: " << root.getChainedPriority())
    LOG4CPLUS_FATAL(root, "test: " << test.getChainedPriority())
    LOG4CPLUS_FATAL(root, "test.subtest: " << subTest.getChainedPriority())

    LOG4CPLUS_FATAL(root, "\nSetting test.subtest to NULL")
    subTest.setPriority(0);
    LOG4CPLUS_FATAL(root, "root: " << root.getChainedPriority())
    LOG4CPLUS_FATAL(root, "test: " << test.getChainedPriority())
    LOG4CPLUS_FATAL(root, "test.subtest: " << subTest.getChainedPriority() << '\n')

    writeLogMessage();
    cout << "Returned from writeLogMessage()..." << endl;
    }
    cout << "REALLY exiting main()..." << endl;

    return 0;
}


