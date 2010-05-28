// Module:  Log4CPLUS
// File:    loglevel.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright 2001-2010 Tad E. Smith
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

#include <log4cplus/loglevel.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>
#include <algorithm>

using namespace log4cplus;
using namespace log4cplus::helpers;

#define _ALL_STRING LOG4CPLUS_TEXT("ALL")
#define _TRACE_STRING LOG4CPLUS_TEXT("TRACE")
#define _DEBUG_STRING LOG4CPLUS_TEXT("DEBUG")
#define _INFO_STRING LOG4CPLUS_TEXT("INFO")
#define _WARN_STRING LOG4CPLUS_TEXT("WARN")
#define _ERROR_STRING LOG4CPLUS_TEXT("ERROR")
#define _FATAL_STRING LOG4CPLUS_TEXT("FATAL")
#define _OFF_STRING LOG4CPLUS_TEXT("OFF")
#define _NOTSET_STRING LOG4CPLUS_TEXT("NOTSET")
#define _UNKNOWN_STRING LOG4CPLUS_TEXT("UNKNOWN")

#define GET_TO_STRING_NODE static_cast<ToStringNode*>(this->toStringMethods)
#define GET_FROM_STRING_NODE static_cast<FromStringNode*>(this->fromStringMethods)



//////////////////////////////////////////////////////////////////////////////
// file LOCAL definitions
//////////////////////////////////////////////////////////////////////////////

namespace {
    class ToStringNode {
    public:
        ToStringNode(LogLevelToStringMethod m) : method(m), next(0) {}

        LogLevelToStringMethod method;
        ToStringNode* next;
    };
    
    
    class FromStringNode {
    public:
        FromStringNode(StringToLogLevelMethod m) : method(m), next(0) {}

        StringToLogLevelMethod method;
        FromStringNode* next;
    };
    
    
    static
    log4cplus::tstring
    defaultLogLevelToStringMethod(LogLevel ll) {
        switch(ll) {
            case OFF_LOG_LEVEL:     return _OFF_STRING;
            case FATAL_LOG_LEVEL:   return _FATAL_STRING;
            case ERROR_LOG_LEVEL:   return _ERROR_STRING;
            case WARN_LOG_LEVEL:    return _WARN_STRING;
            case INFO_LOG_LEVEL:    return _INFO_STRING;
            case DEBUG_LOG_LEVEL:   return _DEBUG_STRING;
            case TRACE_LOG_LEVEL:   return _TRACE_STRING;
            //case ALL_LOG_LEVEL:     return _ALL_STRING;
            case NOT_SET_LOG_LEVEL: return _NOTSET_STRING;
        };
        
        return tstring();
    }
    
    
    static
    LogLevel
    defaultStringToLogLevelMethod(const log4cplus::tstring& arg) {
        log4cplus::tstring s = log4cplus::helpers::toUpper(arg);
        
        if(s == _ALL_STRING)   return ALL_LOG_LEVEL;
        if(s == _TRACE_STRING) return TRACE_LOG_LEVEL;
        if(s == _DEBUG_STRING) return DEBUG_LOG_LEVEL;
        if(s == _INFO_STRING)  return INFO_LOG_LEVEL;
        if(s == _WARN_STRING)  return WARN_LOG_LEVEL;
        if(s == _ERROR_STRING) return ERROR_LOG_LEVEL;
        if(s == _FATAL_STRING) return FATAL_LOG_LEVEL;
        if(s == _OFF_STRING)   return OFF_LOG_LEVEL;
        
        return NOT_SET_LOG_LEVEL;
    }
    
}



//////////////////////////////////////////////////////////////////////////////
// public static methods
//////////////////////////////////////////////////////////////////////////////

LogLevelManager&
log4cplus::getLogLevelManager() 
{
    static LogLevelManager singleton;
    return singleton;
}



//////////////////////////////////////////////////////////////////////////////
// log4cplus::LogLevelManager ctors and dtor
//////////////////////////////////////////////////////////////////////////////

LogLevelManager::LogLevelManager() 
: toStringMethods(new ToStringNode(defaultLogLevelToStringMethod)),
  fromStringMethods(new FromStringNode(defaultStringToLogLevelMethod))
{
}



LogLevelManager::~LogLevelManager() 
{
    ToStringNode* toStringTmp = GET_TO_STRING_NODE;
    while(toStringTmp) {
        ToStringNode* tmp = toStringTmp;
        toStringTmp = toStringTmp->next;
        delete tmp;
    }
    
    FromStringNode* fromStringTmp = GET_FROM_STRING_NODE;
    while(fromStringTmp) {
        FromStringNode* tmp = fromStringTmp;
        fromStringTmp = fromStringTmp->next;
        delete tmp;
    }
}



//////////////////////////////////////////////////////////////////////////////
// log4cplus::LogLevelManager public methods
//////////////////////////////////////////////////////////////////////////////

log4cplus::tstring 
LogLevelManager::toString(LogLevel ll) const
{
    ToStringNode* toStringTmp = GET_TO_STRING_NODE;
    while(toStringTmp) {
        tstring ret = toStringTmp->method(ll);
        if(! ret.empty ()) {
            return ret;
        }
        toStringTmp = toStringTmp->next;
    }
    
    return _UNKNOWN_STRING;
}



LogLevel 
LogLevelManager::fromString(const log4cplus::tstring& s) const
{
    FromStringNode* fromStringTmp = GET_FROM_STRING_NODE;
    while(fromStringTmp) {
        LogLevel ret = fromStringTmp->method(s);
        if(ret != NOT_SET_LOG_LEVEL) {
            return ret;
        }
        fromStringTmp = fromStringTmp->next;
    }
    
    return NOT_SET_LOG_LEVEL;
}



void 
LogLevelManager::pushToStringMethod(LogLevelToStringMethod newToString)
{
    ToStringNode* toStringTmp = GET_TO_STRING_NODE;
    while(1) {
        if(toStringTmp->next) {
            toStringTmp = toStringTmp->next;
        }
        else {
            toStringTmp->next = new ToStringNode(newToString);
            break;
        }
    }
}



void 
LogLevelManager::pushFromStringMethod(StringToLogLevelMethod newFromString)
{
    FromStringNode* fromStringTmp = GET_FROM_STRING_NODE;
    while(1) {
        if(fromStringTmp->next) {
            fromStringTmp = fromStringTmp->next;
        }
        else {
            fromStringTmp->next = new FromStringNode(newFromString);
            break;
        }
    }
}
        
