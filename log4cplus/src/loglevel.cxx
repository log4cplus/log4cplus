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
// Revision 1.4  2003/04/19 07:22:18  tcsmith
// Removed all LogLevelToSysLogMethod methods.
//
// Revision 1.3  2003/04/18 21:07:05  tcsmith
// Converted from std::string to log4cplus::tstring.
//
// Revision 1.2  2003/04/03 02:03:09  tcsmith
// Changed defaultLogLevelToSysLogMethod() so that it returns -1 for unknow
// LogLevels.
//
// Revision 1.1  2003/04/03 01:42:19  tcsmith
// Renamed from priority.cxx
//

#include <log4cplus/loglevel.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>
#include <algorithm>

using namespace log4cplus;
using namespace log4cplus::helpers;

#define _TRACE_STRING LOG4CPLUS_TEXT("TRACE")
#define _DEBUG_STRING LOG4CPLUS_TEXT("DEBUG")
#define _INFO_STRING LOG4CPLUS_TEXT("INFO")
#define _WARN_STRING LOG4CPLUS_TEXT("WARN")
#define _ERROR_STRING LOG4CPLUS_TEXT("ERROR")
#define _FATAL_STRING LOG4CPLUS_TEXT("FATAL")
#define _NOTSET_STRING LOG4CPLUS_TEXT("NOTSET")
#define _UNKNOWN_STRING LOG4CPLUS_TEXT("UNKNOWN")

const log4cplus::tstring& 
log4cplus::defaultLogLevelToStringMethod(LogLevel ll)
{
    static log4cplus::tstring FATAL_LOG_LEVEL_STR(_FATAL_STRING);
    static log4cplus::tstring ERROR_LOG_LEVEL_STR(_ERROR_STRING);
    static log4cplus::tstring WARN_LOG_LEVEL_STR(_WARN_STRING);
    static log4cplus::tstring INFO_LOG_LEVEL_STR(_INFO_STRING);
    static log4cplus::tstring DEBUG_LOG_LEVEL_STR(_DEBUG_STRING);
    static log4cplus::tstring TRACE_LOG_LEVEL_STR(_TRACE_STRING);
    static log4cplus::tstring NOT_SET_LOG_LEVEL_STR(_NOTSET_STRING);
    static log4cplus::tstring UNKNOWN(_UNKNOWN_STRING);

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
log4cplus::defaultStringToLogLevelMethod(const log4cplus::tstring& arg)
{
    log4cplus::tstring s = log4cplus::helpers::toupper(arg);

    if(s == _TRACE_STRING) return TRACE_LOG_LEVEL;
    if(s == _DEBUG_STRING) return DEBUG_LOG_LEVEL;
    if(s == _INFO_STRING)  return INFO_LOG_LEVEL;
    if(s == _WARN_STRING)  return WARN_LOG_LEVEL;
    if(s == _ERROR_STRING) return ERROR_LOG_LEVEL;
    if(s == _FATAL_STRING) return FATAL_LOG_LEVEL;

    return NOT_SET_LOG_LEVEL;
}



LogLevelManager::LogLevelManager() 
: toStringMethod(defaultLogLevelToStringMethod),
  fromStringMethod(defaultStringToLogLevelMethod)
{
}


LogLevelManager&
log4cplus::getLogLevelManager() 
{
    static LogLevelManager singleton;
    return singleton;
}


