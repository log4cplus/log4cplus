// -*- C++ -*-
#ifndef CUSTOMLOGLEVEL_HEADER
#define CUSTOMLOGLEVEL_HEADER

#include <log4cplus/loglevel.h>
#include <log4cplus/loggingmacros.h>

const log4cplus::LogLevel CRITICAL_LOG_LEVEL = 45000;

#define LOG4CPLUS_CRITICAL(logger, logEvent)                            \
    if(logger.isEnabledFor(CRITICAL_LOG_LEVEL)) {                       \
        log4cplus::tostringstream _log4cplus_buf;                       \
        _log4cplus_buf << logEvent;                                     \
	logger.forcedLog(CRITICAL_LOG_LEVEL, _log4cplus_buf.str(), __FILE__, __LINE__); \
    }

#endif // CUSTOMLOGLEVEL_HEADER
