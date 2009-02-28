// Module:  Log4CPLUS
// File:    loglog.cxx
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

#include <log4cplus/streams.h>
#include <log4cplus/helpers/loglog.h>

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;


namespace
{

static tchar const PREFIX[] = LOG4CPLUS_TEXT("log4cplus: ");
static tchar const WARN_PREFIX[] = LOG4CPLUS_TEXT("log4cplus:WARN ");
static tchar const ERR_PREFIX[] = LOG4CPLUS_TEXT("log4cplus:ERROR ");

} // namespace



///////////////////////////////////////////////////////////////////////////////
// static methods
///////////////////////////////////////////////////////////////////////////////

SharedObjectPtr<LogLog>
LogLog::getLogLog()
{
    static SharedObjectPtr<LogLog> singleton(new LogLog());
    return singleton;
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::helpers::LogLog ctor and dtor
///////////////////////////////////////////////////////////////////////////////

LogLog::LogLog()
 : mutex(LOG4CPLUS_MUTEX_CREATE),
   debugEnabled(false),
   quietMode(false)
{
}


LogLog::~LogLog()
{
    LOG4CPLUS_MUTEX_FREE( mutex );
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::helpers::LogLog public methods
///////////////////////////////////////////////////////////////////////////////

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
LogLog::debug(const log4cplus::tstring& msg)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        if(debugEnabled && !quietMode) {
             tcout << PREFIX << msg << endl;
        }
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
}


void
LogLog::warn(const log4cplus::tstring& msg)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        if(quietMode) return;

        tcerr << WARN_PREFIX << msg << endl;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
}


void
LogLog::error(const log4cplus::tstring& msg)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        if(quietMode) return;

        tcerr << ERR_PREFIX << msg << endl;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
}


