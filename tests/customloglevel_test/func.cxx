#include "customloglevel.h"
#include <log4cplus/loggingmacros.h>
#include <log4cplus/helpers/loglog.h>


void
writeLogMessage()
{
    {
        log4cplus::Logger subTest = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("test.subtest"));
        subTest.log(log4cplus::FATAL_LOG_LEVEL, LOG4CPLUS_TEXT("Entering writeLogMessage()..."));
        LOG4CPLUS_CRITICAL(subTest,
            LOG4CPLUS_TEXT("writeLogMessage()- This is a message from a different file"));
        subTest.log(log4cplus::FATAL_LOG_LEVEL, LOG4CPLUS_TEXT("Exiting writeLogMessage()..."));
    }
    log4cplus::helpers::LogLog::getLogLog()->warn(LOG4CPLUS_TEXT("REALLY exiting writeLogMessage()..."));
}
