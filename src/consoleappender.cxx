// Module:  Log4CPLUS
// File:    consoleappender.cxx
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

#include <log4cplus/layout.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggingevent.h>
#include <iostream>

using namespace std;
using namespace log4cplus::helpers;

log4cplus::ConsoleAppender::ConsoleAppender()
{
}


log4cplus::ConsoleAppender::ConsoleAppender(log4cplus::helpers::Properties properties)
 : Appender(properties)
{
}


log4cplus::ConsoleAppender::~ConsoleAppender()
{
    destructorImpl();
}


void 
log4cplus::ConsoleAppender::close()
{
    getLogLog().debug("Entering ConsoleAppender::close()..");
}



// Normally, append() methods do not need to be locked since they are
// called by doAppend() which performs the locking.  However, this locks
// on the LogLog instance, so we don't have multiple threads writing to
// cout and cerr
void
log4cplus::ConsoleAppender::append(const spi::InternalLoggingEvent& event)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( getLogLog().mutex )
        layout->formatAndAppend(cout, event);
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}


