// Module:  Log4CPLUS
// File:    loggingevent.cxx
// Created: 6/2003
// Author:  Tad E. Smith
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

#include <log4cplus/spi/loggingevent.h>


using namespace log4cplus;
using namespace log4cplus::spi;


#define LOG4CPLUS_DEFAULT_TYPE 1


///////////////////////////////////////////////////////////////////////////////
// InternalLoggingEvent dtor
///////////////////////////////////////////////////////////////////////////////

InternalLoggingEvent::~InternalLoggingEvent()
{
}



///////////////////////////////////////////////////////////////////////////////
// InternalLoggingEvent static methods
///////////////////////////////////////////////////////////////////////////////

unsigned int
InternalLoggingEvent::getDefaultType()
{
    return LOG4CPLUS_DEFAULT_TYPE;
}



///////////////////////////////////////////////////////////////////////////////
// InternalLoggingEvent implementation
///////////////////////////////////////////////////////////////////////////////

const log4cplus::tstring& 
InternalLoggingEvent::getMessage() const
{
    return message;
}


unsigned int
InternalLoggingEvent::getType() const
{
    return LOG4CPLUS_DEFAULT_TYPE;
}



std::auto_ptr<InternalLoggingEvent>
InternalLoggingEvent::clone() const
{
    std::auto_ptr<InternalLoggingEvent> tmp(new InternalLoggingEvent(*this));
    return tmp;
}



log4cplus::spi::InternalLoggingEvent&
InternalLoggingEvent::operator=(const log4cplus::spi::InternalLoggingEvent& rhs)
{
    if(this == &rhs) return *this;

    message = rhs.message;
    loggerName = rhs.loggerName;
    ll = rhs.ll;
    ndc = rhs.getNDC();
    thread = rhs.getThread();
    timestamp = rhs.timestamp;
    file = rhs.file;
    line = rhs.line;
    threadCached = true;
    ndcCached = true;

    return *this;
}


