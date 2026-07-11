#include <log4cplus/initializer.h>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#if defined (__ANDROID__)
#include <log4cplus/androidappender.h>
#endif

extern "C" void
log4cplus_consumer_smoke_test()
{
    static log4cplus::Initializer initializer;
    log4cplus::Logger logger
        = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("consumer"));

#if defined (__ANDROID__)
    log4cplus::SharedAppenderPtr appender (
        new log4cplus::AndroidAppender (LOG4CPLUS_TEXT ("log4cplus-consumer")));
    logger.addAppender (appender);
#endif

    LOG4CPLUS_INFO(logger, LOG4CPLUS_TEXT("ordinary log message"));
    LOG4CPLUS_INFO_FORMAT(logger, "formatted log message: {}", 23);
}
