// Module:  Log4CPLUS
// File:    loggingmacros.h
// Created: 8/2003
// Author:  Tad E. Smith
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

/** @file 
 * This header defines the logging macros. */

#ifndef _LOG4CPLUS_LOGGING_MACROS_HEADER_
#define _LOG4CPLUS_LOGGING_MACROS_HEADER_

#if defined(LOG4CPLUS_DISABLE_FATAL) && !defined(LOG4CPLUS_DISABLE_ERROR)
#define LOG4CPLUS_DISABLE_ERROR
#endif
#if defined(LOG4CPLUS_DISABLE_ERROR) && !defined(LOG4CPLUS_DISABLE_WARN)
#define LOG4CPLUS_DISABLE_WARN
#endif
#if defined(LOG4CPLUS_DISABLE_WARN) && !defined(LOG4CPLUS_DISABLE_INFO)
#define LOG4CPLUS_DISABLE_INFO
#endif
#if defined(LOG4CPLUS_DISABLE_INFO) && !defined(LOG4CPLUS_DISABLE_DEBUG)
#define LOG4CPLUS_DISABLE_DEBUG
#endif
#if defined(LOG4CPLUS_DISABLE_DEBUG) && !defined(LOG4CPLUS_DISABLE_TRACE)
#define LOG4CPLUS_DISABLE_TRACE
#endif


/**
 * @def LOG4CPLUS_TRACE(logger, logEvent)  This macro creates a TraceLogger 
 * to log a TRACE_LOG_LEVEL message to <code>logger</code> upon entry and
 * exiting of a method.  
 * <code>logEvent</code> will be streamed into an <code>ostream</code>.
 */
#if !defined(LOG4CPLUS_DISABLE_TRACE)
#define LOG4CPLUS_TRACE_METHOD(logger, logEvent) \
    log4cplus::TraceLogger _log4cplus_trace_logger(logger, logEvent, __FILE__, __LINE__);
#define LOG4CPLUS_TRACE(logger, logEvent) \
    do { \
        if(logger.isEnabledFor(log4cplus::TRACE_LOG_LEVEL)) { \
            log4cplus::tostringstream _log4cplus_buf; \
            _log4cplus_buf << logEvent; \
            logger.forcedLog(log4cplus::TRACE_LOG_LEVEL, _log4cplus_buf.str(), __FILE__, __LINE__); \
        } \
    } while(0);
#define LOG4CPLUS_TRACE_STR(logger, logEvent) \
    do { \
        if(logger.isEnabledFor(log4cplus::TRACE_LOG_LEVEL)) { \
            logger.forcedLog(log4cplus::TRACE_LOG_LEVEL, logEvent, __FILE__, __LINE__); \
        } \
    } while(0);
#else
#define LOG4CPLUS_TRACE_METHOD(logger, logEvent)
#define LOG4CPLUS_TRACE(logger, logEvent)
#define LOG4CPLUS_TRACE_STR(logger, logEvent)
#endif

/**
 * @def LOG4CPLUS_DEBUG(logger, logEvent)  This macro is used to log a
 * DEBUG_LOG_LEVEL message to <code>logger</code>.  
 * <code>logEvent</code> will be streamed into an <code>ostream</code>.
 */
#if !defined(LOG4CPLUS_DISABLE_DEBUG)
#define LOG4CPLUS_DEBUG(logger, logEvent) \
    do { \
        if(logger.isEnabledFor(log4cplus::DEBUG_LOG_LEVEL)) { \
            log4cplus::tostringstream _log4cplus_buf; \
            _log4cplus_buf << logEvent; \
            logger.forcedLog(log4cplus::DEBUG_LOG_LEVEL, _log4cplus_buf.str(), __FILE__, __LINE__); \
        } \
    } while(0);
#define LOG4CPLUS_DEBUG_STR(logger, logEvent) \
    do { \
        if(logger.isEnabledFor(log4cplus::DEBUG_LOG_LEVEL)) { \
            logger.forcedLog(log4cplus::DEBUG_LOG_LEVEL, logEvent, __FILE__, __LINE__); \
        } \
    } while(0);
#else
#define LOG4CPLUS_DEBUG(logger, logEvent) 
#define LOG4CPLUS_DEBUG_STR(logger, logEvent)
#endif

/**
 * @def LOG4CPLUS_INFO(logger, logEvent)  This macro is used to log a
 * INFO_LOG_LEVEL message to <code>logger</code>.  
 * <code>logEvent</code> will be streamed into an <code>ostream</code>.
 */
#if !defined(LOG4CPLUS_DISABLE_INFO)
#define LOG4CPLUS_INFO(logger, logEvent) \
    do { \
        if(logger.isEnabledFor(log4cplus::INFO_LOG_LEVEL)) { \
            log4cplus::tostringstream _log4cplus_buf; \
            _log4cplus_buf << logEvent; \
            logger.forcedLog(log4cplus::INFO_LOG_LEVEL, _log4cplus_buf.str(), __FILE__, __LINE__); \
        } \
    } while(0);
#define LOG4CPLUS_INFO_STR(logger, logEvent) \
    do { \
        if(logger.isEnabledFor(log4cplus::INFO_LOG_LEVEL)) { \
            logger.forcedLog(log4cplus::INFO_LOG_LEVEL, logEvent, __FILE__, __LINE__); \
        } \
    } while(0);
#else
#define LOG4CPLUS_INFO(logger, logEvent)
#define LOG4CPLUS_INFO_STR(logger, logEvent)
#endif

/**
 * @def LOG4CPLUS_WARN(logger, logEvent)  This macro is used to log a
 * WARN_LOG_LEVEL message to <code>logger</code>.  
 * <code>logEvent</code> will be streamed into an <code>ostream</code>.
 */
#if !defined(LOG4CPLUS_DISABLE_WARN)
#define LOG4CPLUS_WARN(logger, logEvent) \
    do { \
        if(logger.isEnabledFor(log4cplus::WARN_LOG_LEVEL)) { \
            log4cplus::tostringstream _log4cplus_buf; \
            _log4cplus_buf << logEvent; \
            logger.forcedLog(log4cplus::WARN_LOG_LEVEL, _log4cplus_buf.str(), __FILE__, __LINE__); \
        } \
    } while(0);
#define LOG4CPLUS_WARN_STR(logger, logEvent) \
    do { \
        if(logger.isEnabledFor(log4cplus::WARN_LOG_LEVEL)) { \
            logger.forcedLog(log4cplus::WARN_LOG_LEVEL, logEvent, __FILE__, __LINE__); \
        } \
    } while(0);
#else
#define LOG4CPLUS_WARN(logger, logEvent)
#define LOG4CPLUS_WARN_STR(logger, logEvent)
#endif

/**
 * @def LOG4CPLUS_ERROR(logger, logEvent)  This macro is used to log a
 * ERROR_LOG_LEVEL message to <code>logger</code>.  
 * <code>logEvent</code> will be streamed into an <code>ostream</code>.
 */
#if !defined(LOG4CPLUS_DISABLE_ERROR)
#define LOG4CPLUS_ERROR(logger, logEvent) \
    do { \
        if(logger.isEnabledFor(log4cplus::ERROR_LOG_LEVEL)) { \
            log4cplus::tostringstream _log4cplus_buf; \
            _log4cplus_buf << logEvent; \
            logger.forcedLog(log4cplus::ERROR_LOG_LEVEL, _log4cplus_buf.str(), __FILE__, __LINE__); \
        } \
    } while(0);
#define LOG4CPLUS_ERROR_STR(logger, logEvent) \
    do { \
        if(logger.isEnabledFor(log4cplus::ERROR_LOG_LEVEL)) { \
            logger.forcedLog(log4cplus::ERROR_LOG_LEVEL, logEvent, __FILE__, __LINE__); \
        } \
    } while(0);
#else
#define LOG4CPLUS_ERROR(logger, logEvent)
#define LOG4CPLUS_ERROR_STR(logger, logEvent)
#endif

/**
 * @def LOG4CPLUS_FATAL(logger, logEvent)  This macro is used to log a
 * FATAL_LOG_LEVEL message to <code>logger</code>.  
 * <code>logEvent</code> will be streamed into an <code>ostream</code>.
 */
#if !defined(LOG4CPLUS_DISABLE_FATAL)
#define LOG4CPLUS_FATAL(logger, logEvent) \
    do { \
        if(logger.isEnabledFor(log4cplus::FATAL_LOG_LEVEL)) { \
            log4cplus::tostringstream _log4cplus_buf; \
            _log4cplus_buf << logEvent; \
            logger.forcedLog(log4cplus::FATAL_LOG_LEVEL, _log4cplus_buf.str(), __FILE__, __LINE__); \
        } \
    } while(0);
#define LOG4CPLUS_FATAL_STR(logger, logEvent) \
    do { \
        if(logger.isEnabledFor(log4cplus::FATAL_LOG_LEVEL)) { \
            logger.forcedLog(log4cplus::FATAL_LOG_LEVEL, logEvent, __FILE__, __LINE__); \
        } \
    } while(0);
#else
#define LOG4CPLUS_FATAL(logger, logEvent)
#define LOG4CPLUS_FATAL_STR(logger, logEvent) 
#endif

#endif /* _LOG4CPLUS_LOGGING_MACROS_HEADER_ */

