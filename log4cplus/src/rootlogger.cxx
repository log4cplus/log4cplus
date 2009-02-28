// Module:  Log4CPLUS
// File:    rootlogger.cxx
// Created: 4/2003
// Author:  Tad E. Smith
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

#include <log4cplus/spi/rootlogger.h>

using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::spi;



//////////////////////////////////////////////////////////////////////////////
// RootLogger Constructor
//////////////////////////////////////////////////////////////////////////////

RootLogger::RootLogger(Hierarchy& h, LogLevel ll)
: LoggerImpl(LOG4CPLUS_TEXT("root"), h)
{
    setLogLevel(ll);
}



//////////////////////////////////////////////////////////////////////////////
// Logger Methods
//////////////////////////////////////////////////////////////////////////////

LogLevel 
RootLogger::getChainedLogLevel() const
{
    return ll;
}


void 
RootLogger::setLogLevel(LogLevel ll)
{
    if(ll == NOT_SET_LOG_LEVEL) {
        getLogLog().error(LOG4CPLUS_TEXT("You have tried to set NOT_SET_LOG_LEVEL to root."));
    }
    else {
        LoggerImpl::setLogLevel(ll);
    }
}

