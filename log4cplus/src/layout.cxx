// Module:  Log4CPLUS
// File:    layout.cxx
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

#include <log4cplus/layout.h>
#include <log4cplus/spi/loggingevent.h>


using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::spi;

#define BUFFER_SIZE 20


///////////////////////////////////////////////////////////////////////////////
// public methods
///////////////////////////////////////////////////////////////////////////////

std::string 
log4cplus::getFormattedTime(time_t time, const std::string& fmt)
{
    char buffer[BUFFER_SIZE];
    int len = strftime(buffer, BUFFER_SIZE, fmt.c_str(), gmtime(&time));
    buffer[len] = '\0';
    return buffer;
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::SimpleLayout public methods
///////////////////////////////////////////////////////////////////////////////

void
SimpleLayout::formatAndAppend(std::ostream& output, 
                              const log4cplus::spi::InternalLoggingEvent& event)
{
    output << llmCache.toString(event.ll) << " - " << event.message << std::endl;
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::TTCCLayout ctors and dtor
///////////////////////////////////////////////////////////////////////////////

TTCCLayout::TTCCLayout()
: dateFormat("%m-%d-%y %H:%M:%S")
{
}


TTCCLayout::TTCCLayout(log4cplus::helpers::Properties properties)
: Layout(properties),
  dateFormat("%m-%d-%y %H:%M:%S")
{
}


TTCCLayout::~TTCCLayout()
{
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::TTCCLayout public methods
///////////////////////////////////////////////////////////////////////////////

void
TTCCLayout::formatAndAppend(std::ostream& output, 
                            const log4cplus::spi::InternalLoggingEvent& event)
{
    output << getFormattedTime(event.timestamp, dateFormat) << " ["
           << event.thread << "] "
           << llmCache.toString(event.ll) << " "
           << event.loggerName << " <"
           << event.ndc << "> - "
           << event.message
           << std::endl;
}




