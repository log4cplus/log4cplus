// Module:  Log4CPLUS
// File:    loglog.cxx
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

#include <log4cplus/helpers/loglog.h>
#include <iostream>

using namespace std;
using namespace log4cplus::helpers;


namespace {
    class _static_LogLog_initializer {
    public:
        _static_LogLog_initializer() { log4cplus::helpers::getLogLog(); }
    } initializer;
}


LogLog&
log4cplus::helpers::getLogLog()
{
    static LogLog *instance = NULL;

    if(instance == NULL) {
        instance = new LogLog();
    }
    return *instance;
}


LogLog::LogLog()
 : mutex(LOG4CPLUS_MUTEX_CREATE),
   debugEnabled(false),
   quietMode(false),
   PREFIX("log4cplus: "),
   WARN_PREFIX("log4cplus:WARN "),
   ERR_PREFIX("log4cplus:ERROR ")
{
}


LogLog::~LogLog()
{
    LOG4CPLUS_MUTEX_FREE( mutex );
}


void
LogLog::setInternalDebugging(bool enabled)
{
    debugEnabled = enabled;
}

void
LogLog::setQuietMode(bool quietModeVal)
{
    quietMode = quietModeVal;
}

void
LogLog::debug(const std::string& msg)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        if(debugEnabled && !quietMode) {
             cout << PREFIX << msg << endl;
        }
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}


void
LogLog::warn(const std::string& msg)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        if(quietMode) return;

        cerr << WARN_PREFIX << msg << endl;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}


void
LogLog::error(const std::string& msg)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        if(quietMode) return;

        cerr << ERR_PREFIX << msg << endl;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}


