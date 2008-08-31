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
// $Log: not supported by cvs2svn $
// Revision 1.7  2003/08/08 05:32:22  tcsmith
// Changed the #if checks to look for _WIN32 and not WIN32.
//
// Revision 1.6  2003/06/23 20:56:43  tcsmith
// Modified to support the changes in the spi::InternalLoggingEvent class.
//
// Revision 1.5  2003/06/06 17:04:31  tcsmith
// Changed the ctor to take a 'const' Properties object.
//
// Revision 1.4  2003/06/03 20:19:41  tcsmith
// Modified the close() method to set "closed = true;".
//
// Revision 1.3  2003/05/21 22:19:39  tcsmith
// Changed getSysLogLevel(), so that it has a default return value to remove
// a compiler warning message.
//
// Revision 1.2  2003/04/19 21:35:31  tcsmith
// Added WIN32 check.
//
// Revision 1.1  2003/04/19 07:25:34  tcsmith
// Initial version.
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

log4cplus::SysLogAppender::SysLogAppender(const tstring& ident)
: ident(ident)
{
    ::openlog(ident.c_str(), 0, 0);
}


log4cplus::SysLogAppender::SysLogAppender(const Properties properties)
: Appender(properties)
{
    ident = properties.getProperty( LOG4CPLUS_TEXT("ident") );
    ::openlog(ident.c_str(), 0, 0);
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
    getLogLog().debug("Entering SysLogAppender::close()...");
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex )
        ::closelog();
        closed = true;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
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
        ::syslog(level, buf.str().c_str());
    }
}

#endif // defined(LOG4CPLUS_HAVE_SYSLOG_H)

