// Module:  Log4CPLUS
// File:    priority.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright (C) The Apache Software Foundation. All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

#include <log4cplus/priority.h>
#include <log4cplus/helpers/loglog.h>
#include <algorithm>

using namespace log4cplus;
using namespace log4cplus::helpers;


namespace {
    // Function object to turn a lower case character into an upper case one
    class ToUpper {
    public:
        void operator()(char& c){c = toupper(c);}
    };
}



Priority
Priority::toPriority(int val, const Priority& defaultPriority)
{
    switch(val) {
    case DEBUG_PRI:
    case INFO_PRI:
    case WARN_PRI:
    case ERROR_PRI:
    case FATAL_PRI:
        return Priority(static_cast<PriorityLevel>(val));

    default: 
        return defaultPriority;
    }
}


Priority
Priority::toPriority(const std::string& sArg, const Priority& defaultPriority)
{
    if(sArg.length() == 0) {
        return defaultPriority;
    }

    std::string s(sArg);
    std::for_each(s.begin(), s.end(), ToUpper());

    if(s == "DEBUG") return Priority(DEBUG_PRI);
    if(s == "INFO")  return Priority(INFO_PRI);
    if(s == "WARN")  return Priority(WARN_PRI);
    if(s == "ERROR") return Priority(ERROR_PRI);
    if(s == "FATAL") return Priority(FATAL_PRI);

    return defaultPriority;
}


PriorityList 
Priority::getAllPossiblePriorities()
{
    PriorityList ret;
    ret.push_back( Priority(FATAL_PRI) );
    ret.push_back( Priority(ERROR_PRI) );
    ret.push_back( Priority(WARN_PRI) );
    ret.push_back( Priority(INFO_PRI) );
    ret.push_back( Priority(DEBUG_PRI) );

    return ret;
}


bool 
Priority::isGreaterOrEqual(const Priority* p) const
{
    if(p == 0) return false;
    else return level >= p->level;
}



int 
Priority::getSyslogEquivalent() const
{
    switch(level) {
        case DEBUG_PRI: return 7;
        case INFO_PRI:  return 6;
        case WARN_PRI:  return 4;
        case ERROR_PRI: return 3;
        case FATAL_PRI: return 0;
    }

    return 7;
}



std::string 
Priority::toString() const
{
    switch(level) {
        case DEBUG_PRI: return "DEBUG";
        case INFO_PRI:  return "INFO";
        case WARN_PRI:  return "WARN";
        case ERROR_PRI: return "ERROR";
        case FATAL_PRI: return "FATAL";
    }

    return "UNKNOWN";
}

