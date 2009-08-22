// Module:  Log4CPLUS
// File:    loglog.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright 2001-2009 Tad E. Smith
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

#include <log4cplus/streams.h>
#include <log4cplus/helpers/loglog.h>
#include <ostream>


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

LogLog *
LogLog::getLogLog()
{
    static LogLog singleton;
    return &singleton;
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
    logging_worker (tcout, &LogLog::get_debug_mode, PREFIX, msg);
}


void
LogLog::debug(tchar const * msg)
{
    logging_worker (tcout, &LogLog::get_debug_mode, PREFIX, msg);
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
