// Module:  Log4CPLUS
// File:    layout.cxx
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
#include <log4cplus/spi/loggingevent.h>


using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::spi;

#define BUFFER_SIZE 20


std::string 
log4cplus::getFormattedTime(time_t time, const std::string& fmt)
{
    char buffer[BUFFER_SIZE];
    int len = strftime(buffer, BUFFER_SIZE, fmt.c_str(), gmtime(&time));
    buffer[len] = '\0';
    return buffer;
}



void
SimpleLayout::formatAndAppend(std::ostream& output, const log4cplus::spi::InternalLoggingEvent& event)
{
    output << event.priority << " - " << event.message << std::endl;
}



TTCCLayout::TTCCLayout()
: dateFormat("%m-%d-%y %H:%M:%S")
{
}


TTCCLayout::~TTCCLayout()
{
}


void
TTCCLayout::formatAndAppend(std::ostream& output, const log4cplus::spi::InternalLoggingEvent& event)
{
    output << getFormattedTime(event.timestamp, dateFormat) << " ["
           << event.thread << "] "
           << event.priority << " "
           << event.categoryName << " <"
           << event.ndc << "> - "
           << event.message;
    if(event.file != NULL) {
        output << " [" << event.file << ':' << event.line << ']';
    }
    output << std::endl;
}




