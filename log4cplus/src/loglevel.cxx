// Module:  Log4CPLUS
// File:    loglevel.cxx
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
// $Log: not supported by cvs2svn $
// Revision 1.1  2003/04/03 01:42:19  tcsmith
// Renamed from priority.cxx
//

#include <log4cplus/loglevel.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>
#include <algorithm>

using namespace log4cplus;
using namespace log4cplus::helpers;


const std::string& 
log4cplus::defaultLogLevelToStringMethod(LogLevel ll)
{
    static std::string FATAL_LOG_LEVEL_STR("FATAL");
    static std::string ERROR_LOG_LEVEL_STR("ERROR");
    static std::string WARN_LOG_LEVEL_STR("WARN");
    static std::string INFO_LOG_LEVEL_STR("INFO");
    static std::string DEBUG_LOG_LEVEL_STR("DEBUG");
    static std::string TRACE_LOG_LEVEL_STR("TRACE");
    static std::string NOT_SET_LOG_LEVEL_STR("NOTSET");
    static std::string UNKNOWN("UNKNOWN");

    switch(ll) {
        case FATAL_LOG_LEVEL: return FATAL_LOG_LEVEL_STR;
        case ERROR_LOG_LEVEL: return ERROR_LOG_LEVEL_STR;
        case WARN_LOG_LEVEL: return WARN_LOG_LEVEL_STR;
        case INFO_LOG_LEVEL: return INFO_LOG_LEVEL_STR;
        case DEBUG_LOG_LEVEL: return DEBUG_LOG_LEVEL_STR;
        case TRACE_LOG_LEVEL: return TRACE_LOG_LEVEL_STR;
        case NOT_SET_LOG_LEVEL: return NOT_SET_LOG_LEVEL_STR;
    };

    return UNKNOWN;
}



LogLevel 
log4cplus::defaultStringToLogLevelMethod(const std::string& arg)
{
    std::string s = log4cplus::helpers::toupper(arg);

    if(s == "TRACE") return TRACE_LOG_LEVEL;
    if(s == "DEBUG") return DEBUG_LOG_LEVEL;
    if(s == "INFO")  return INFO_LOG_LEVEL;
    if(s == "WARN")  return WARN_LOG_LEVEL;
    if(s == "ERROR") return ERROR_LOG_LEVEL;
    if(s == "FATAL") return FATAL_LOG_LEVEL;

    return NOT_SET_LOG_LEVEL;
}



int 
log4cplus::defaultLogLevelToSysLogMethod(LogLevel ll)
{
    switch(ll) {
        case FATAL_LOG_LEVEL: return 0;
        case ERROR_LOG_LEVEL: return 3;
        case WARN_LOG_LEVEL: return 4;
        case INFO_LOG_LEVEL: return 5;
        case DEBUG_LOG_LEVEL: return 6;
        case TRACE_LOG_LEVEL: return -1;
    };

    return -1;
}



LogLevelManager::LogLevelManager() 
: toStringMethod(defaultLogLevelToStringMethod),
  fromStringMethod(defaultStringToLogLevelMethod),
  toSysLogMethod(defaultLogLevelToSysLogMethod)
{
}


LogLevelManager&
log4cplus::getLogLevelManager() 
{
    static LogLevelManager instance;
    return instance;
}


