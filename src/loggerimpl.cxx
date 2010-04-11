// Module:  Log4CPLUS
// File:    loggerimpl.cxx
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
// Revision 1.6  2003/06/13 17:49:29  tcsmith
// Changed to use the old style C headers.
//
// Revision 1.5  2003/05/26 19:19:56  tcsmith
// Fixed Bug #741515 - "LogLog is causing a core on HP".
//
// Revision 1.4  2003/04/19 23:16:37  tcsmith
// Removed TABs from the file.
//
// Revision 1.3  2003/04/19 23:15:47  tcsmith
// Fixed UNICODE support.
//
// Revision 1.2  2003/04/18 21:26:16  tcsmith
// Converted from std::string to log4cplus::tstring.
//
// Revision 1.1  2003/04/03 01:44:01  tcsmith
// Renamed from categoryimpl.cxx
//

#include <log4cplus/spi/loggerimpl.h>
#include <log4cplus/appender.h>
#include <log4cplus/hierarchy.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/spi/rootlogger.h>
#include <stdexcept>

using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::spi;



//////////////////////////////////////////////////////////////////////////////
// Logger Constructors and Destructor
//////////////////////////////////////////////////////////////////////////////
LoggerImpl::LoggerImpl(const log4cplus::tstring& name, Hierarchy& h)
  : name(name),
    ll(NOT_SET_LOG_LEVEL),
    parent(NULL),
    additive(true), 
    hierarchy(h)
{
}


LoggerImpl::~LoggerImpl() 
{ 
}


//////////////////////////////////////////////////////////////////////////////
// Logger Methods
//////////////////////////////////////////////////////////////////////////////

void 
LoggerImpl::callAppenders(const InternalLoggingEvent& event)
{
    int writes = 0;
    for(const LoggerImpl* c = this; c != NULL; c=c->parent.get()) {
        writes += c->appendLoopOnAppenders(event);
        if(!c->additive) {
            break;
        }
    }

    // No appenders in hierarchy, warn user only once.
    if(!hierarchy.emittedNoAppenderWarning && writes == 0) {
        getLogLog().error(  LOG4CPLUS_TEXT("No appenders could be found for logger (") 
                          + getName() 
                          + LOG4CPLUS_TEXT(")."));
        getLogLog().error(LOG4CPLUS_TEXT("Please initialize the log4cplus system properly."));
        hierarchy.emittedNoAppenderWarning = true;
    }
}


void 
LoggerImpl::closeNestedAppenders()
{
    SharedAppenderPtrList appenders = getAllAppenders();
    for(SharedAppenderPtrList::iterator it=appenders.begin(); it!=appenders.end(); ++it)
    {
        (*it)->close();
    }
}


bool 
LoggerImpl::isEnabledFor(LogLevel ll) const
{
    if(hierarchy.disableValue >= ll) {
        return false;
    }
    return ll >= getChainedLogLevel();
}


void 
LoggerImpl::log(LogLevel ll, 
                const log4cplus::tstring& message,
                const char* file, 
                int line)
{
    if(isEnabledFor(ll)) {
        forcedLog(ll, message, file, line);
    }
}



LogLevel 
LoggerImpl::getChainedLogLevel() const
{
    for(const LoggerImpl *c=this; c != NULL; c=c->parent.get()) {
        if(c == NULL) {
            getLogLog().error(LOG4CPLUS_TEXT("LoggerImpl::getChainedLogLevel()- Internal error: NullPointer"));
            helpers::throwNullPointerException(__FILE__, __LINE__);
        }
        if(c->ll != NOT_SET_LOG_LEVEL) {
            return c->ll;
        }
    }

    getLogLog().error( LOG4CPLUS_TEXT("LoggerImpl::getChainedLogLevel()- No valid LogLevel found") );
    throw std::runtime_error("No valid LogLevel found");
}


Hierarchy& 
LoggerImpl::getHierarchy() const
{ 
    return hierarchy; 
}


bool 
LoggerImpl::getAdditivity() const
{ 
    return additive; 
}


void 
LoggerImpl::setAdditivity(bool additive) 
{ 
    this->additive = additive; 
}


void 
LoggerImpl::forcedLog(LogLevel ll,
                      const log4cplus::tstring& message,
                      const char* file, 
                      int line)
{
    callAppenders(spi::InternalLoggingEvent(this->getName(), ll, message, file, line));
}



