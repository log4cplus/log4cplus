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
// $Log: not supported by cvs2svn $
// Revision 1.4  2003/04/12 13:51:08  tcsmith
// No longer dynamically allocate the object in the "singleton" method.
//
// Revision 1.3  2003/04/03 01:02:02  tcsmith
// Standardized the formatting.
//

#include <log4cplus/streams.h>
#include <log4cplus/helpers/loglog.h>

using namespace std;
using namespace log4cplus::helpers;


///////////////////////////////////////////////////////////////////////////////
// File LOCAL definitions
///////////////////////////////////////////////////////////////////////////////

namespace {
    class _static_LogLog_initializer {
    public:
        _static_LogLog_initializer() { log4cplus::helpers::getLogLog(); }
    } initializer;
}



///////////////////////////////////////////////////////////////////////////////
// public methods
///////////////////////////////////////////////////////////////////////////////

LogLog&
log4cplus::helpers::getLogLog()
{
    static LogLog singleton;
    return singleton;
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::helpers::LogLog ctor and dtor
///////////////////////////////////////////////////////////////////////////////

LogLog::LogLog()
 : mutex(LOG4CPLUS_MUTEX_CREATE),
   debugEnabled(false),
   quietMode(false),
   PREFIX( LOG4CPLUS_TEXT("log4cplus: ") ),
   WARN_PREFIX( LOG4CPLUS_TEXT("log4cplus:WARN ") ),
   ERR_PREFIX( LOG4CPLUS_TEXT("log4cplus:ERROR ") )
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
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}


void
LogLog::warn(const log4cplus::tstring& msg)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        if(quietMode) return;

        tcerr << WARN_PREFIX << msg << endl;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}


void
LogLog::error(const log4cplus::tstring& msg)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        if(quietMode) return;

        tcerr << ERR_PREFIX << msg << endl;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}


