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
// Revision 1.10  2003/06/04 19:38:35  tcsmith
// Added a use_gmtime flag to TTCCLayout to specifiy whether dates should be
// logged using gmtime or localtime function calls.
//
// Revision 1.9  2003/06/04 18:59:00  tcsmith
// Modified to use the "time" function defined in the timehelper.h header.
//
// Revision 1.8  2003/05/21 22:14:46  tcsmith
// Fixed compiler warning: "conversion from 'size_t' to 'int', possible loss
// of data".
//
// Revision 1.7  2003/04/19 23:04:31  tcsmith
// Fixed UNICODE support.
//
// Revision 1.6  2003/04/18 21:19:42  tcsmith
// Converted from std::string to log4cplus::tstring.
//
// Revision 1.5  2003/04/03 00:58:59  tcsmith
// Standardized the formatting.
//

#include <log4cplus/layout.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/helpers/timehelper.h>
#include <log4cplus/spi/loggingevent.h>


using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::spi;

#define BUFFER_SIZE 40


///////////////////////////////////////////////////////////////////////////////
// public methods
///////////////////////////////////////////////////////////////////////////////

log4cplus::tstring
log4cplus::getFormattedTime(time_t time, const log4cplus::tstring& fmt,
                            bool use_gmtime)
{
    tchar buffer[BUFFER_SIZE];
    struct tm tmp;
    struct tm* tPtr = 0;
   
    if(use_gmtime) {
        tPtr = log4cplus::helpers::gmtime(&time, &tmp);
    }
    else {
        tPtr = log4cplus::helpers::localtime(&time, &tmp);
    }

    size_t len = log4cplus::helpers::strftime(buffer, 
                                              BUFFER_SIZE, 
                                              fmt.c_str(), 
                                              tPtr);

    buffer[len] = '\0';
    return tstring(buffer);
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::SimpleLayout public methods
///////////////////////////////////////////////////////////////////////////////

void
SimpleLayout::formatAndAppend(log4cplus::tostream& output, 
                              const log4cplus::spi::InternalLoggingEvent& event)
{
    output << llmCache.toString(event.ll) 
           << LOG4CPLUS_TEXT(" - ")
           << event.message 
           << std::endl;
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::TTCCLayout ctors and dtor
///////////////////////////////////////////////////////////////////////////////

TTCCLayout::TTCCLayout(bool use_gmtime)
: dateFormat( LOG4CPLUS_TEXT("%m-%d-%y %H:%M:%S") ),
  use_gmtime(use_gmtime)
{
}


TTCCLayout::TTCCLayout(const log4cplus::helpers::Properties& properties)
: Layout(properties),
  dateFormat( LOG4CPLUS_TEXT("%m-%d-%y %H:%M:%S") ),
  use_gmtime(true)
{
    if(properties.exists( LOG4CPLUS_TEXT("DateFormat") )) {
        dateFormat  = properties.getProperty( LOG4CPLUS_TEXT("DateFormat") );
    }

    tstring tmp = properties.getProperty( LOG4CPLUS_TEXT("Use_gmtime") );
    use_gmtime = (tolower(tmp) == LOG4CPLUS_TEXT("true"));
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
    output << getFormattedTime(event.timestamp, dateFormat, use_gmtime) 
           << LOG4CPLUS_TEXT(" [")
           << event.thread 
           << LOG4CPLUS_TEXT("] ")
           << llmCache.toString(event.ll) 
           << LOG4CPLUS_TEXT(" ")
           << event.loggerName 
           << LOG4CPLUS_TEXT(" <")
           << event.ndc 
           << LOG4CPLUS_TEXT("> - ")
           << event.message
           << std::endl;
}




