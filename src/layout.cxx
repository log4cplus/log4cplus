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

#include <log4cplus/layout.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/helpers/timehelper.h>
#include <log4cplus/spi/loggingevent.h>


using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::spi;


///////////////////////////////////////////////////////////////////////////////
// log4cplus::SimpleLayout public methods
///////////////////////////////////////////////////////////////////////////////

void
SimpleLayout::formatAndAppend(log4cplus::tostream& output, 
                              const log4cplus::spi::InternalLoggingEvent& event)
{
    output << llmCache.toString(event.getLogLevel()) 
           << LOG4CPLUS_TEXT(" - ")
           << event.getMessage() 
           << LOG4CPLUS_TEXT("\n");
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::TTCCLayout ctors and dtor
///////////////////////////////////////////////////////////////////////////////

TTCCLayout::TTCCLayout(bool use_gmtime)
: dateFormat( LOG4CPLUS_TEXT("%m-%d-%y %H:%M:%S,%q") ),
  use_gmtime(use_gmtime)
{
}


TTCCLayout::TTCCLayout(const log4cplus::helpers::Properties& properties)
: Layout(properties),
  dateFormat( LOG4CPLUS_TEXT("%m-%d-%y %H:%M:%S,%q") ),
  use_gmtime(false)
{
    if(properties.exists( LOG4CPLUS_TEXT("DateFormat") )) {
        dateFormat  = properties.getProperty( LOG4CPLUS_TEXT("DateFormat") );
    }

    tstring tmp = properties.getProperty( LOG4CPLUS_TEXT("Use_gmtime") );
    use_gmtime = (toLower(tmp) == LOG4CPLUS_TEXT("true"));
}


TTCCLayout::~TTCCLayout()
{
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::TTCCLayout public methods
///////////////////////////////////////////////////////////////////////////////

void
TTCCLayout::formatAndAppend(log4cplus::tostream& output, 
                            const log4cplus::spi::InternalLoggingEvent& event)
{
    output << event.getTimestamp().getFormattedTime(dateFormat, use_gmtime) 
           << LOG4CPLUS_TEXT(" [")
           << event.getThread()
           << LOG4CPLUS_TEXT("] ")
           << llmCache.toString(event.getLogLevel()) 
           << LOG4CPLUS_TEXT(" ")
           << event.getLoggerName()
           << LOG4CPLUS_TEXT(" <")
           << event.getNDC() 
           << LOG4CPLUS_TEXT("> - ")
           << event.getMessage()
           << LOG4CPLUS_TEXT("\n");
}




