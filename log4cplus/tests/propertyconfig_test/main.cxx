
#include <log4cplus/category.h>
#include <log4cplus/propertyconfig.h>
#include <log4cplus/helpers/loglog.h>

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
        PropertyConfigurator configurator("log4cplus.properties");
	configurator.configure();

	Category::getRoot().warn("Testing....");
    }
    catch(...) {
        cout << "Exception..." << endl;
        Category::getRoot().fatal("Exception occured...");
    }

    cout << "Exiting main()..." << endl;
    return 0;
}

