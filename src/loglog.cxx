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


namespace log4cplus { namespace helpers {

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
    logging_worker (tcout, &LogLog::get_debug_mode, ERR_PREFIX, msg);
}


void
LogLog::debug(tchar const * msg)
{
    logging_worker (tcout, &LogLog::get_debug_mode, ERR_PREFIX, msg);
}


void
LogLog::warn(const log4cplus::tstring& msg)
{
    logging_worker (tcerr, &LogLog::get_quiet_mode, WARN_PREFIX, msg);
}


void
LogLog::warn(tchar const * msg)
{
    logging_worker (tcerr, &LogLog::get_quiet_mode, WARN_PREFIX, msg);
}


void
LogLog::error(const log4cplus::tstring& msg)
{
    logging_worker (tcerr, &LogLog::get_quiet_mode, ERR_PREFIX, msg);
}


void
LogLog::error(tchar const * msg)
{
    logging_worker (tcerr, &LogLog::get_quiet_mode, ERR_PREFIX, msg);
}


bool
LogLog::get_quiet_mode () const
{
    return quietMode;
}


bool
LogLog::get_debug_mode () const
{
    return debugEnabled && ! quietMode;
}


template <typename StringType>
void
LogLog::logging_worker (tostream & os, bool (LogLog:: * cond) () const,
    tchar const * prefix, StringType const & msg)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        if (! (this->*cond) ())
            return;

        os << prefix << msg << std::endl;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
}


} } // namespace log4cplus { namespace helpers {
