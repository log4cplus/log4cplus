// Module:  Log4CPLUS
// File:    loglevel.h
// Created: 6/2001
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
 * This header defines the LogLevel type.
 */

#ifndef LOG4CPLUS_LOGLEVEL_HEADER_
#define LOG4CPLUS_LOGLEVEL_HEADER_

#include <log4cplus/config.h>
#include <log4cplus/streams.h>
#include <log4cplus/tstring.h>

#include <vector>

namespace log4cplus {

    /** 
     * \typedef int LogLevel
     * Defines the minimum set of priorities recognized by the system,
     * that is {@link #FATAL_LOG_LEVEL}, {@link #ERROR_LOG_LEVEL}, {@link
     * #WARN_LOG_LEVEL}, {@link #INFO_LOG_LEVEL}, {@link #DEBUG_LOG_LEVEL}, 
     * and {@link #TRACE_LOG_LEVEL}.
     */
    typedef int LogLevel;

    /** \var const LogLevel FATAL_LOG_LEVEL
     * The <code>FATAL_LOG_LEVEL</code> LogLevel designates very severe error
     * events that will presumably lead the application to abort.  */
    const LogLevel FATAL_LOG_LEVEL   = 50000;

    /** \var const LogLevel ERROR_LOG_LEVEL
     * The <code>ERROR_LOG_LEVEL</code> LogLevel designates error events that
     * might still allow the application to continue running. */
    const LogLevel ERROR_LOG_LEVEL   = 40000;

    /** \var const LogLevel WARN_LOG_LEVEL
     * The <code>WARN_LOG_LEVEL</code> LogLevel designates potentially harmful 
     * situations. */
    const LogLevel WARN_LOG_LEVEL    = 30000;

    /** \var const LogLevel INFO_LOG_LEVEL
     * The <code>INFO_LOG_LEVEL</code> LogLevel designates informational 
     * messages  that highlight the progress of the application at 
     * coarse-grained  level. */
    const LogLevel INFO_LOG_LEVEL    = 20000;

    /** \var const LogLevel DEBUG_LOG_LEVEL
     * The <code>DEBUG_LOG_LEVEL</code> LogLevel designates fine-grained
     * informational events that are most useful to debug an application. */
    const LogLevel DEBUG_LOG_LEVEL   = 10000;

    /** \var const LogLevel TRACE_LOG_LEVEL
     * The <code>TRACE_LOG_LEVEL</code> LogLevel is used to "trace" entry
     * and exiting of methods. */
    const LogLevel TRACE_LOG_LEVEL   = 0;

    /** \var const LogLevel NOT_SET_LOG_LEVEL
     * The <code>NOT_SET_LOG_LEVEL</code> LogLevel is used to indicated that
     * no particular LogLevel is desired and that the default should be used.
     */
    const LogLevel NOT_SET_LOG_LEVEL = -1;


    /** 
     * This method type defined the signature of methods that convert LogLevels
     * into strings. */
    typedef const log4cplus::tstring& (*LogLevelToStringMethod)(LogLevel);


    typedef LogLevel (*StringToLogLevelMethod)(const log4cplus::tstring&);

    LOG4CPLUS_EXPORT const log4cplus::tstring& defaultLogLevelToStringMethod(LogLevel);
    LOG4CPLUS_EXPORT LogLevel defaultStringToLogLevelMethod(const log4cplus::tstring&);

    class LOG4CPLUS_EXPORT LogLevelManager {
    public:
        LogLevelManager();

        const log4cplus::tstring& toString(LogLevel ll) const 
             { return toStringMethod(ll); }
        LogLevel fromString(const log4cplus::tstring& s) const 
             { return fromStringMethod(s); }

        LogLevelToStringMethod setToStringMapper(LogLevelToStringMethod newToString) {
            LogLevelToStringMethod tmp = toStringMethod;
            toStringMethod = newToString;
            return tmp;
        }

        StringToLogLevelMethod setFromStringMapper(StringToLogLevelMethod newFromString) {
            StringToLogLevelMethod tmp = fromStringMethod;
            fromStringMethod = newFromString;
            return tmp;
        }

    private:
        LogLevelToStringMethod toStringMethod;
        StringToLogLevelMethod fromStringMethod;
    };

    LOG4CPLUS_EXPORT LogLevelManager& getLogLevelManager();

}


#endif // LOG4CPLUS_LOGLEVEL_HEADER_

