// Module:  Log4CPLUS
// File:    loggingmacros.h
// Created: 8/2003
// Author:  Tad E. Smith
//
//
// Copyright 2003-2009 Tad E. Smith
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/** @file 
 * This header defines the logging macros. */

#ifndef _LOG4CPLUS_LOGGING_MACROS_HEADER_
#define _LOG4CPLUS_LOGGING_MACROS_HEADER_

#include <log4cplus/config.hxx>
#include <log4cplus/streams.h>


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


#if defined (LOG4CPLUS_SINGLE_THREADED)

namespace log4cplus
{

extern LOG4CPLUS_EXPORT tostringstream _macros_oss;

LOG4CPLUS_EXPORT void _clear_tostringstream (tostringstream &);

} // namespace log4cplus


#define LOG4CPLUS_MACRO_BODY(logger, logEvent, logLevel)                \
    do {                                                                \
        if((logger).isEnabledFor(log4cplus::logLevel##_LOG_LEVEL)) {    \
            log4cplus::_clear_tostringstream (log4cplus::_macros_oss);  \
            log4cplus::_macros_oss << logEvent;                         \
            (logger).forcedLog(log4cplus::logLevel##_LOG_LEVEL,         \
                log4cplus::_macros_oss.str(), __FILE__, __LINE__);      \
        }                                                               \
    } while (0)


#else // defined (LOG4CPLUS_SINGLE_THREADED)

#define LOG4CPLUS_MACRO_BODY(logger, logEvent, logLevel)                \
    do {                                                                \
        if((logger).isEnabledFor(log4cplus::logLevel##_LOG_LEVEL)) {    \
            log4cplus::tostringstream _log4cplus_buf;                   \
            _log4cplus_buf << logEvent;                                 \
            (logger).forcedLog(log4cplus::logLevel##_LOG_LEVEL,         \
                _log4cplus_buf.str(), __FILE__, __LINE__);              \
        }                                                               \
    } while (0)


#endif // defined (LOG4CPLUS_SINGLE_THREADED)

#define LOG4CPLUS_MACRO_STR_BODY(logger, logEvent, logLevel)            \
    do {                                                                \
        if((logger).isEnabledFor(log4cplus::logLevel##_LOG_LEVEL)) {    \
            (logger).forcedLog(log4cplus::logLevel##_LOG_LEVEL,         \
                logEvent, __FILE__, __LINE__);                          \
        }                                                               \
    } while(0)


/**
 * @def LOG4CPLUS_TRACE(logger, logEvent)  This macro creates a TraceLogger 
 * to log a TRACE_LOG_LEVEL message to <code>logger</code> upon entry and
 * exiting of a method.  
 * <code>logEvent</code> will be streamed into an <code>ostream</code>.
 */
#if !defined(LOG4CPLUS_DISABLE_TRACE)
#define LOG4CPLUS_TRACE_METHOD(logger, logEvent)                        \
    log4cplus::TraceLogger _log4cplus_trace_logger(logger, logEvent,    \
                                                   __FILE__, __LINE__);
#define LOG4CPLUS_TRACE(logger, logEvent)                               \
    LOG4CPLUS_MACRO_BODY (logger, logEvent, TRACE)
#define LOG4CPLUS_TRACE_STR(logger, logEvent)                           \
    LOG4CPLUS_MACRO_STR_BODY (logger, logEvent, TRACE)
#else
#define LOG4CPLUS_TRACE_METHOD(logger, logEvent) do { } while (0)
#define LOG4CPLUS_TRACE(logger, logEvent) do { } while (0)
#define LOG4CPLUS_TRACE_STR(logger, logEvent) do { } while (0)
#endif

/**
 * @def LOG4CPLUS_DEBUG(logger, logEvent)  This macro is used to log a
 * DEBUG_LOG_LEVEL message to <code>logger</code>.  
 * <code>logEvent</code> will be streamed into an <code>ostream</code>.
 */
#if !defined(LOG4CPLUS_DISABLE_DEBUG)
#define LOG4CPLUS_DEBUG(logger, logEvent)                               \
    LOG4CPLUS_MACRO_BODY (logger, logEvent, DEBUG)
#define LOG4CPLUS_DEBUG_STR(logger, logEvent)                           \
    LOG4CPLUS_MACRO_STR_BODY (logger, logEvent, DEBUG)
#else
#define LOG4CPLUS_DEBUG(logger, logEvent) do { } while (0)
#define LOG4CPLUS_DEBUG_STR(logger, logEvent) do { } while (0)
#endif

/**
 * @def LOG4CPLUS_INFO(logger, logEvent)  This macro is used to log a
 * INFO_LOG_LEVEL message to <code>logger</code>.  
 * <code>logEvent</code> will be streamed into an <code>ostream</code>.
 */
#if !defined(LOG4CPLUS_DISABLE_INFO)
#define LOG4CPLUS_INFO(logger, logEvent)                                \
    LOG4CPLUS_MACRO_BODY (logger, logEvent, INFO)
#define LOG4CPLUS_INFO_STR(logger, logEvent)                            \
    LOG4CPLUS_MACRO_STR_BODY (logger, logEvent, INFO)
#else
#define LOG4CPLUS_INFO(logger, logEvent) do { } while (0)
#define LOG4CPLUS_INFO_STR(logger, logEvent) do { } while (0)
#endif

/**
 * @def LOG4CPLUS_WARN(logger, logEvent)  This macro is used to log a
 * WARN_LOG_LEVEL message to <code>logger</code>.  
 * <code>logEvent</code> will be streamed into an <code>ostream</code>.
 */
#if !defined(LOG4CPLUS_DISABLE_WARN)
#define LOG4CPLUS_WARN(logger, logEvent)                                \
    LOG4CPLUS_MACRO_BODY (logger, logEvent, WARN)
#define LOG4CPLUS_WARN_STR(logger, logEvent)                            \
    LOG4CPLUS_MACRO_STR_BODY (logger, logEvent, WARN)
#else
#define LOG4CPLUS_WARN(logger, logEvent) do { } while (0)
#define LOG4CPLUS_WARN_STR(logger, logEvent) do { } while (0)
#endif

/**
 * @def LOG4CPLUS_ERROR(logger, logEvent)  This macro is used to log a
 * ERROR_LOG_LEVEL message to <code>logger</code>.  
 * <code>logEvent</code> will be streamed into an <code>ostream</code>.
 */
#if !defined(LOG4CPLUS_DISABLE_ERROR)
#define LOG4CPLUS_ERROR(logger, logEvent)                               \
    LOG4CPLUS_MACRO_BODY (logger, logEvent, ERROR)
#define LOG4CPLUS_ERROR_STR(logger, logEvent)                           \
    LOG4CPLUS_MACRO_STR_BODY (logger, logEvent, ERROR)
#else
#define LOG4CPLUS_ERROR(logger, logEvent) do { } while (0)
#define LOG4CPLUS_ERROR_STR(logger, logEvent) do { } while (0)
#endif

/**
 * @def LOG4CPLUS_FATAL(logger, logEvent)  This macro is used to log a
 * FATAL_LOG_LEVEL message to <code>logger</code>.  
 * <code>logEvent</code> will be streamed into an <code>ostream</code>.
 */
#if !defined(LOG4CPLUS_DISABLE_FATAL)
#define LOG4CPLUS_FATAL(logger, logEvent)                               \
    LOG4CPLUS_MACRO_BODY (logger, logEvent, FATAL)
#define LOG4CPLUS_FATAL_STR(logger, logEvent)                           \
    LOG4CPLUS_MACRO_STR_BODY (logger, logEvent, FATAL)
#else
#define LOG4CPLUS_FATAL(logger, logEvent) do { } while (0)
#define LOG4CPLUS_FATAL_STR(logger, logEvent) do { } while (0)
#endif

#endif /* _LOG4CPLUS_LOGGING_MACROS_HEADER_ */

