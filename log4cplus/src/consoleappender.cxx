// Module:  Log4CPLUS
// File:    consoleappender.cxx
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
// Revision 1.8  2003/06/06 17:04:31  tcsmith
// Changed the ctor to take a 'const' Properties object.
//
// Revision 1.7  2003/06/03 20:19:42  tcsmith
// Modified the close() method to set "closed = true;".
//
// Revision 1.6  2003/05/04 01:29:19  tcsmith
// Added the ability to log to stderr.
//
// Revision 1.5  2003/04/18 21:00:40  tcsmith
// Converted from std::string to log4cplus::tstring.
//
// Revision 1.4  2003/04/03 00:37:26  tcsmith
// Standardized the formatting.
//

#include <log4cplus/layout.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/streams.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/spi/loggingevent.h>

using namespace std;
using namespace log4cplus::helpers;


//////////////////////////////////////////////////////////////////////////////
// log4cplus::ConsoleAppender ctors and dtor
//////////////////////////////////////////////////////////////////////////////

log4cplus::ConsoleAppender::ConsoleAppender(bool logToStdErr)
: logToStdErr(logToStdErr)
{
}



log4cplus::ConsoleAppender::ConsoleAppender(const log4cplus::helpers::Properties properties)
 : Appender(properties)
{
    tstring val = toLower(properties.getProperty(LOG4CPLUS_TEXT("logToStdErr")));
    if(val == LOG4CPLUS_TEXT("true")) {
        logToStdErr = true;
    }
}



log4cplus::ConsoleAppender::~ConsoleAppender()
{
    destructorImpl();
}



//////////////////////////////////////////////////////////////////////////////
// log4cplus::ConsoleAppender public methods
//////////////////////////////////////////////////////////////////////////////

void 
log4cplus::ConsoleAppender::close()
{
    getLogLog().debug(LOG4CPLUS_TEXT("Entering ConsoleAppender::close().."));
    closed = true;
}



//////////////////////////////////////////////////////////////////////////////
// log4cplus::ConsoleAppender protected methods
//////////////////////////////////////////////////////////////////////////////

// Normally, append() methods do not need to be locked since they are
// called by doAppend() which performs the locking.  However, this locks
// on the LogLog instance, so we don't have multiple threads writing to
// tcout and tcerr
void
log4cplus::ConsoleAppender::append(const spi::InternalLoggingEvent& event)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( getLogLog().mutex )
        layout->formatAndAppend((logToStdErr ? tcerr : tcout), event);
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}


