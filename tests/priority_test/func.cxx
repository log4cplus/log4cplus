
#include "log4cplus/logger.h"
#include "log4cplus/helpers/loglog.h"

using namespace log4cplus;
using namespace log4cplus::helpers;

void
writeLogMessage() {
    {
    Logger subTest = Logger::getInstance("test.subtest");
    subTest.log(FATAL_LOG_LEVEL, "Entering writeLogMessage()...");
    LOG4CPLUS_FATAL(subTest, "writeLogMessage()- This is a message from a different file")
    subTest.log(FATAL_LOG_LEVEL, "Exiting writeLogMessage()...");
    }
    LogLog::getLogLog()->warn("REALLY exiting writeLogMessage()...");
}


