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

#include <log4cplus/spi/loggerimpl.h>
#include <log4cplus/appender.h>
#include <log4cplus/hierarchy.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/spi/rootlogger.h>
#include <cassert>
#include <stdexcept>
#include <string>

using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::spi;



//////////////////////////////////////////////////////////////////////////////
// Logger Constructors and Destructor
//////////////////////////////////////////////////////////////////////////////
LoggerImpl::LoggerImpl(const std::string& name, Hierarchy& h)
  : name(name),
    ll(NOT_SET_LOG_LEVEL),
    parent(NULL),
    additive(true), 
    hierarchy(h)
{
}


LoggerImpl::~LoggerImpl() 
{ 
    getLogLog().debug("Destroying Logger: " + name);
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
        getLogLog().error("No appenders could be found for logger (" + 
                          getName() + ").");
        getLogLog().error("Please initialize the log4cplus system properly.");
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
                const std::string& message,
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
            getLogLog().error("LoggerImpl::getChainedLogLevel()- Internal error: NullPointer");
            helpers::throwNullPointerException(__FILE__, __LINE__);
        }
        if(c->ll != NOT_SET_LOG_LEVEL) {
            return c->ll;
        }
    }

    getLogLog().error("LoggerImpl::getChainedLogLevel()- No valid LogLevel found");
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
                      const std::string& message,
                      const char* file, 
                      int line)
{
    callAppenders(spi::InternalLoggingEvent(this->getName(), ll, message, file, line));
}



