// -*- C++ -*-
// Module:  Log4CPLUS
// File:    loggingmacros.h
// Created: 8/2003
// Author:  Tad E. Smith
//
//
// Copyright 2003-2010 Tad E. Smith
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
#include <log4cplus/logger.h>
#include <log4cplus/helpers/snprintf.h>
#include <log4cplus/tracelogger.h>
#include <sstream>


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


namespace log4cplus
{

namespace detail
{


inline
Logger
macros_get_logger (Logger const & logger)
{
    return logger;
}


inline
Logger const &
macros_get_logger (Logger & logger)
{
    return logger;
}


inline
Logger
macros_get_logger (tstring const & logger)
{
    return Logger::getInstance (logger);
}


inline
Logger
macros_get_logger (tchar const * logger)
{
    return Logger::getInstance (logger);
}


LOG4CPLUS_EXPORT void clear_tostringstream (tostringstream &);


LOG4CPLUS_EXPORT log4cplus::tostringstream & get_macro_body_oss ();
LOG4CPLUS_EXPORT log4cplus::helpers::snprintf_buf & get_macro_body_snprintf_buf ();
LOG4CPLUS_EXPORT void macro_forced_log (log4cplus::Logger const &,
    log4cplus::LogLevel, log4cplus::tstring const &, char const *, int,
    char const *);


} // namespace detail

} // namespace log4cplus


#undef LOG4CPLUS_MACRO_FUNCTION
#define LOG4CPLUS_MACRO_FUNCTION() 0 
#if ! defined (LOG4CPLUS_DISABLE_FUNCTION_MACRO)
#  if defined (LOG4CPLUS_HAVE_FUNCSIG_MACRO)
#    undef LOG4CPLUS_MACRO_FUNCTION
#    define LOG4CPLUS_MACRO_FUNCTION() __FUNCSIG__
#  elif defined (LOG4CPLUS_HAVE_PRETTY_FUNCTION_MACRO)
#    undef LOG4CPLUS_MACRO_FUNCTION
#    define LOG4CPLUS_MACRO_FUNCTION() __PRETTY_FUNCTION__
#  elif defined (LOG4CPLUS_HAVE_FUNCTION_MACRO)
#    undef LOG4CPLUS_MACRO_FUNCTION
#    define LOG4CPLUS_MACRO_FUNCTION() __FUNCTION__
#  endif
#endif


#define LOG4CPLUS_MACRO_BODY(logger, logEvent, logLevel)                \
    do {                                                                \
        log4cplus::Logger const & _l                                    \
            = log4cplus::detail::macros_get_logger (logger);            \
        if (_l.isEnabledFor (log4cplus::logLevel##_LOG_LEVEL)) {        \
            log4cplus::tostringstream & _log4cplus_buf                  \
                = log4cplus::detail::get_macro_body_oss ();             \
            _log4cplus_buf << logEvent;                                 \
            log4cplus::detail::macro_forced_log (_l,                    \
                log4cplus::logLevel##_LOG_LEVEL, _log4cplus_buf.str(),  \
                __FILE__, __LINE__, LOG4CPLUS_MACRO_FUNCTION ());       \
        }                                                               \
    } while (0)


#define LOG4CPLUS_MACRO_STR_BODY(logger, logEvent, logLevel)            \
    do {                                                                \
        log4cplus::Logger const & _l                                    \
            = log4cplus::detail::macros_get_logger (logger);            \
        if (_l.isEnabledFor (log4cplus::logLevel##_LOG_LEVEL)) {        \
            log4cplus::detail::macro_forced_log (_l,                    \
                log4cplus::logLevel##_LOG_LEVEL, logEvent,              \
                __FILE__, __LINE__, LOG4CPLUS_MACRO_FUNCTION ());       \
        }                                                               \
    } while(0)

#if defined (LOG4CPLUS_HAVE_C99_VARIADIC_MACROS)
#define LOG4CPLUS_MACRO_FMT_BODY(logger, logLevel, logFmt, ...)         \
    do {                                                                \
        log4cplus::Logger const & _l                                    \
            = log4cplus::detail::macros_get_logger (logger);            \
        if (_l.isEnabledFor (log4cplus::logLevel##_LOG_LEVEL)) {        \
            log4cplus::helpers::snprintf_buf & _snpbuf                  \
                = log4cplus::detail::get_macro_body_snprintf_buf ();    \
            log4cplus::tchar const * _logEvent                          \
                = _snpbuf.print (logFmt, __VA_ARGS__);                  \
            log4cplus::detail::macro_forced_log (_l,                    \
                log4cplus::logLevel##_LOG_LEVEL, _logEvent,             \
                __FILE__, __LINE__, LOG4CPLUS_MACRO_FUNCTION ());       \
        }                                                               \
    } while(0)

#elif defined (LOG4CPLUS_HAVE_GNU_VARIADIC_MACROS)
#define LOG4CPLUS_MACRO_FMT_BODY(logger, logLevel, logFmt, logArgs...)  \
    do {                                                                \
        log4cplus::Logger const & _l                                    \
            = log4cplus::detail::macros_get_logger (logger);            \
        if (_l.isEnabledFor (log4cplus::logLevel##_LOG_LEVEL)) {        \
            log4cplus::helpers::snprintf_buf & _snpbuf                  \
                = log4cplus::detail::get_macro_body_snprintf_buf ();    \
            log4cplus::tchar const * _logEvent                          \
                = _snpbuf.print (logFmt, logArgs);                      \
            log4cplus::detail::macro_forced_log (_l,                    \
                log4cplus::logLevel##_LOG_LEVEL, _logEvent,             \
                __FILE__, __LINE__, LOG4CPLUS_MACRO_FUNCTION ());       \
        }                                                               \
    } while(0)

#endif


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

#if defined (LOG4CPLUS_HAVE_C99_VARIADIC_MACROS)
#define LOG4CPLUS_TRACE_FMT(logger, logFmt, ...)                        \
    LOG4CPLUS_MACRO_FMT_BODY (logger, TRACE, logFmt, __VA_ARGS__)
#elif defined (LOG4CPLUS_HAVE_GNU_VARIADIC_MACROS)
#define LOG4CPLUS_TRACE_FMT(logger, logFmt, logArgs...)                 \
    LOG4CPLUS_MACRO_FMT_BODY(logger, TRACE, logFmt, logArgs)
#endif

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

#if defined (LOG4CPLUS_HAVE_C99_VARIADIC_MACROS)
#define LOG4CPLUS_DEBUG_FMT(logger, logFmt, ...)                        \
    LOG4CPLUS_MACRO_FMT_BODY (logger, DEBUG, logFmt, __VA_ARGS__)
#elif defined (LOG4CPLUS_HAVE_GNU_VARIADIC_MACROS)
#define LOG4CPLUS_DEBUG_FMT(logger, logFmt, logArgs...)                 \
    LOG4CPLUS_MACRO_FMT_BODY(logger, DEBUG, logFmt, logArgs)
#endif

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

#if defined (LOG4CPLUS_HAVE_C99_VARIADIC_MACROS)
#define LOG4CPLUS_INFO_FMT(logger, logFmt, ...)                        \
    LOG4CPLUS_MACRO_FMT_BODY (logger, INFO, logFmt, __VA_ARGS__)
#elif defined (LOG4CPLUS_HAVE_GNU_VARIADIC_MACROS)
#define LOG4CPLUS_INFO_FMT(logger, logFmt, logArgs...)                 \
    LOG4CPLUS_MACRO_FMT_BODY(logger, INFO, logFmt, logArgs)
#endif

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

#if defined (LOG4CPLUS_HAVE_C99_VARIADIC_MACROS)
#define LOG4CPLUS_WARN_FMT(logger, logFmt, ...)                        \
    LOG4CPLUS_MACRO_FMT_BODY (logger, WARN, logFmt, __VA_ARGS__)
#elif defined (LOG4CPLUS_HAVE_GNU_VARIADIC_MACROS)
#define LOG4CPLUS_WARN_FMT(logger, logFmt, logArgs...)                 \
    LOG4CPLUS_MACRO_FMT_BODY(logger, WARN, logFmt, logArgs)
#endif

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

#if defined (LOG4CPLUS_HAVE_C99_VARIADIC_MACROS)
#define LOG4CPLUS_ERROR_FMT(logger, logFmt, ...)                        \
    LOG4CPLUS_MACRO_FMT_BODY (logger, ERROR, logFmt, __VA_ARGS__)
#elif defined (LOG4CPLUS_HAVE_GNU_VARIADIC_MACROS)
#define LOG4CPLUS_ERROR_FMT(logger, logFmt, logArgs...)                 \
    LOG4CPLUS_MACRO_FMT_BODY(logger, ERROR, logFmt, logArgs)
#endif

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

#if defined (LOG4CPLUS_HAVE_C99_VARIADIC_MACROS)
#define LOG4CPLUS_FATAL_FMT(logger, logFmt, ...)                        \
    LOG4CPLUS_MACRO_FMT_BODY (logger, FATAL, logFmt, __VA_ARGS__)
#elif defined (LOG4CPLUS_HAVE_GNU_VARIADIC_MACROS)
#define LOG4CPLUS_FATAL_FMT(logger, logFmt, logArgs...)                 \
    LOG4CPLUS_MACRO_FMT_BODY(logger, FATAL, logFmt, logArgs)
#endif

#else
#define LOG4CPLUS_FATAL(logger, logEvent) do { } while (0)
#define LOG4CPLUS_FATAL_STR(logger, logEvent) do { } while (0)
#endif

#endif /* _LOG4CPLUS_LOGGING_MACROS_HEADER_ */
