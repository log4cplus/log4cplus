// Module:  Log4CPLUS
// File:    categoryimpl.cxx
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

#include <log4cplus/spi/categoryimpl.h>
#include <log4cplus/appender.h>
#include <log4cplus/hierarchy.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/spi/rootcategory.h>
#include <cassert>
#include <stdexcept>
#include <string>

using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::spi;



//////////////////////////////////////////////////////////////////////////////
// Category Constructors and Destructor
//////////////////////////////////////////////////////////////////////////////
CategoryImpl::CategoryImpl(const std::string& name, Hierarchy& h)
  : name(name),
    priority(NULL),
    parent(NULL),
    additive(true), 
    hierarchy(h)
{
}


CategoryImpl::~CategoryImpl() 
{ 
    getLogLog().debug("Destroying Category: " + name);
    delete this->priority;
}


//////////////////////////////////////////////////////////////////////////////
// Category Methods
//////////////////////////////////////////////////////////////////////////////

void 
CategoryImpl::callAppenders(const InternalLoggingEvent& event)
{
    int writes = 0;
    for(const CategoryImpl* c = this; c != NULL; c=c->parent.get()) {
        writes += c->appendLoopOnAppenders(event);
        if(!c->additive) {
            break;
        }
    }

    // No appenders in hierarchy, warn user only once.
    if(!hierarchy.emittedNoAppenderWarning && writes == 0) {
        getLogLog().error("No appenders could be found for category (" + 
                          getName() + ").");
        getLogLog().error("Please initialize the log4cplus system properly.");
        hierarchy.emittedNoAppenderWarning = true;
    }
}


void 
CategoryImpl::closeNestedAppenders()
{
    SharedAppenderPtrList appenders = getAllAppenders();
    for(SharedAppenderPtrList::iterator it=appenders.begin(); it!=appenders.end(); ++it)
    {
        (*it)->close();
    }
}


bool 
CategoryImpl::isEnabledFor(const Priority& priority) const
{
    if(hierarchy.disableValue >= priority.level) {
        return false;
    }
    return priority.isGreaterOrEqual(getChainedPriority());
}


void 
CategoryImpl::log(const Priority& priority, 
                  const std::string& message,
                  const char* file, 
                  int line)
{
    if(isEnabledFor(priority)) {
        forcedLog(priority, message, file, line);
    }
}



Priority 
CategoryImpl::getChainedPriority() const
{
    for(const CategoryImpl *c = this; c != NULL; c=c->parent.get()) {
        if(c == NULL) {
            getLogLog().error("CategoryImpl::getChainedPriority()- Internal error: NullPointer");
            helpers::throwNullPointerException(__FILE__, __LINE__);
        }
        if(c->priority != NULL) {
            return *(c->priority);
        }
    }

    getLogLog().error("CategoryImpl::getChainedPriority()- No valid Priority found");
    throw std::runtime_error("No valid Priority found");
}


void 
CategoryImpl::setPriority(const Priority* p)
{
    if(this->priority != NULL) {
        delete this->priority;
        this->priority = NULL;
    }

    if(p != NULL) {
        this->priority = new Priority( *p );
    }
}


Hierarchy& 
CategoryImpl::getHierarchy() const
{ 
    return hierarchy; 
}


bool 
CategoryImpl::getAdditivity() const
{ 
    return additive; 
}


void 
CategoryImpl::setAdditivity(bool additive) 
{ 
    this->additive = additive; 
}


void 
CategoryImpl::forcedLog(const Priority& priority, 
                        const std::string& message,
                        const char* file, 
                        int line)
{
    callAppenders(spi::InternalLoggingEvent(this->getName(), priority, message, file, line));
}



