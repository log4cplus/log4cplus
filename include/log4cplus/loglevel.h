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

/** @file */

#ifndef LOG4CPLUS_LOGLEVEL_HEADER_
#define LOG4CPLUS_LOGLEVEL_HEADER_

#include <log4cplus/config.h>
#include <iostream>
#include <string>
#include <vector>

namespace log4cplus {

    typedef int LogLevel;

    const LogLevel FATAL_LOG_LEVEL   = 50000;
    const LogLevel ERROR_LOG_LEVEL   = 40000;
    const LogLevel WARN_LOG_LEVEL    = 30000;
    const LogLevel INFO_LOG_LEVEL    = 20000;
    const LogLevel DEBUG_LOG_LEVEL   = 10000;
    const LogLevel TRACE_LOG_LEVEL   = 0;
    const LogLevel NOT_SET_LOG_LEVEL = -1;


    typedef const std::string& (*LogLevelToStringMethod)(LogLevel);
    typedef LogLevel (*StringToLogLevelMethod)(const std::string&);
    typedef int (*LogLevelToSysLogMethod)(LogLevel);

    const std::string& defaultLogLevelToStringMethod(LogLevel);
    LogLevel defaultStringToLogLevelMethod(const std::string&);
    int defaultLogLevelToSysLogMethod(LogLevel);

    class LogLevelManager {
    public:
        LogLevelManager();

        const std::string& toString(LogLevel ll) const { return toStringMethod(ll); }
        LogLevel fromString(const std::string& s) const { return fromStringMethod(s); }
        int toSysLog(LogLevel ll) const { return toSysLogMethod(ll); }

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

        LogLevelToSysLogMethod setStringMapper(LogLevelToSysLogMethod newToSysLog) {
            LogLevelToSysLogMethod tmp = toSysLogMethod;
            toSysLogMethod = newToSysLog;
            return tmp;
        }

    private:
        LogLevelToStringMethod toStringMethod;
        StringToLogLevelMethod fromStringMethod;
        LogLevelToSysLogMethod toSysLogMethod;
    };

    LogLevelManager& getLogLevelManager();

}


#endif // LOG4CPLUS_LOGLEVEL_HEADER_

