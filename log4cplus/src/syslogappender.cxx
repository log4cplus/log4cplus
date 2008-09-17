// Module:  Log4CPLUS
// File:    syslogappender.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

#include <log4cplus/syslogappender.h>
#if defined(LOG4CPLUS_HAVE_SYSLOG_H) && !defined(_WIN32)

#include <log4cplus/streams.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggingevent.h>

#include <syslog.h>

using namespace std;
using namespace log4cplus::helpers;



///////////////////////////////////////////////////////////////////////////////
// log4cplus::SysLogAppender ctors and dtor
///////////////////////////////////////////////////////////////////////////////

log4cplus::SysLogAppender::SysLogAppender(const tstring& id)
: ident(id)
{
    ::openlog(LOG4CPLUS_TSTRING_TO_STRING (ident).c_str(), 0, 0);
}


log4cplus::SysLogAppender::SysLogAppender(const Properties properties)
: Appender(properties)
{
    ident = properties.getProperty( LOG4CPLUS_TEXT("ident") );
    ::openlog(LOG4CPLUS_TSTRING_TO_STRING (ident).c_str(), 0, 0);
}


log4cplus::SysLogAppender::~SysLogAppender()
{
    destructorImpl();
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::SysLogAppender public methods
///////////////////////////////////////////////////////////////////////////////

void 
log4cplus::SysLogAppender::close()
{
    getLogLog().debug(LOG4CPLUS_TEXT("Entering SysLogAppender::close()..."));
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex )
        ::closelog();
        closed = true;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::SysLogAppender protected methods
///////////////////////////////////////////////////////////////////////////////

int
log4cplus::SysLogAppender::getSysLogLevel(const LogLevel& ll) const
{
    if(ll < DEBUG_LOG_LEVEL) {
        return -1;
    }
    else if(ll < INFO_LOG_LEVEL) {
        return LOG_DEBUG;
    }
    else if(ll < WARN_LOG_LEVEL) {
        return LOG_INFO;
    }
    else if(ll < ERROR_LOG_LEVEL) {
        return LOG_WARNING;
    }
    else if(ll < FATAL_LOG_LEVEL) {
        return LOG_ERR;
    }
    else if(ll == FATAL_LOG_LEVEL) {
        return LOG_CRIT;
    }

    return LOG_ALERT;  // ll > FATAL_LOG_LEVEL
}


// This method does not need to be locked since it is called by
// doAppend() which performs the locking
void
log4cplus::SysLogAppender::append(const spi::InternalLoggingEvent& event)
{
    int level = getSysLogLevel(event.getLogLevel());
    if(level != -1) {
        log4cplus::tostringstream buf;
        layout->formatAndAppend(buf, event);
        ::syslog(level, LOG4CPLUS_TSTRING_TO_STRING(buf.str()).c_str());
    }
}

#endif // defined(LOG4CPLUS_HAVE_SYSLOG_H)

