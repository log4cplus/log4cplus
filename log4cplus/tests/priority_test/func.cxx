
#include "log4cplus/category.h"
#include "log4cplus/helpers/loglog.h"

using namespace log4cplus;
using namespace log4cplus::helpers;

void
writeLogMessage() {
    {
    Category subTest = Category::getInstance("test.subtest");
    subTest.fatal("Entering writeLogMessage()...");
    LOG4CPLUS_FATAL(subTest, "writeLogMessage()- This is a message from a different file")
    subTest.fatal("Exiting writeLogMessage()...");
    }
    getLogLog().warn("REALLY exiting writeLogMessage()...");
}


