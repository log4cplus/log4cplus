
#include <log4cplus/category.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/ndc.h>
#include <log4cplus/helpers/loglog.h>
#include <iostream>
#include <string>

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;

int
main()
{
    cout << "Entering main()..." << endl;
    getLogLog().setInternalDebugging(true);
    try {
        SharedObjectPtr<Appender> append_1(new ConsoleAppender());
        append_1->setName("First");
        Category::getRoot().addAppender(append_1);

        Category cat = Category::getInstance("test");
        cout << "Category: " << cat.getName() << endl;
        getNDC().push("tsmith");
        LOG4CPLUS_DEBUG(cat, "This is a short test...")

        getNDC().push("password");
        LOG4CPLUS_DEBUG(cat, "This should have my password now");

        getNDC().pop();
        LOG4CPLUS_DEBUG(cat, "This should NOT have my password now");

        getNDC().pop();
        cout << "Just returned from pop..." << endl;
        LOG4CPLUS_DEBUG(cat, "There should be no NDC...");

        getNDC().push("tsmith");
        getNDC().push("password");
        LOG4CPLUS_DEBUG(cat, "This should have my password now");
        getNDC().remove();
        LOG4CPLUS_DEBUG(cat, "There should be no NDC...");
    }
    catch(...) {
        cout << "Exception..." << endl;
        Category::getRoot().fatal("Exception occured...");
    }

    cout << "Exiting main()..." << endl;
    return 0;
}

