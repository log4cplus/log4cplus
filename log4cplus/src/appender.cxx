// Module:  Log4CPLUS
// File:    appender.cxx
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

#include <log4cplus/appender.h>
#include <log4cplus/layout.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/pointer.h>
#include <log4cplus/spi/loggingevent.h>

using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::spi;


void
OnlyOnceErrorHandler::error(const std::string& err) 
{
    if(firstTime) {
        getLogLog().error(err);
        firstTime = false;
    }
}


Appender::Appender()
 : layout(new SimpleLayout()),
   name(""),
   threshold(NULL),
   errorHandler(new OnlyOnceErrorHandler()),
   closed(false)
{
}


void 
Appender::destructorImpl()
{
    // An appender might be closed then destroyed. There is no
    // point in closing twice.
    if(closed) {
        return;
    }

    getLogLog().debug("Destroying appender named ["+name+"].");
    close();
    closed = true;
}


ErrorHandler* 
Appender::getErrorHandler()
{
    return errorHandler.get();
}


Layout* 
Appender::getLayout()
{
    return layout.get();
}


std::string 
Appender::getName()
{
    return name;
}


const Priority* 
Appender::getThreshold()
{
    return threshold;
}


bool 
Appender::isAsSevereAsThreshold(const Priority* priority)
{
    if(priority == NULL) {
        return false;
    }
    return ((threshold == NULL) || priority->isGreaterOrEqual(threshold));
}


void 
Appender::doAppend(const log4cplus::spi::InternalLoggingEvent& event)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex )
        if(closed) {
            getLogLog().error("Attempted to append to closed appender named ["+name+"].");
            return;
        }

        if(!isAsSevereAsThreshold(event.priority)) {
            return;
        }

        append(event);
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}


void 
Appender::setErrorHandler(std::auto_ptr<ErrorHandler> eh)
{
    if(eh.get() == NULL) {
        // We do not throw exception here since the cause is probably a
        // bad config file.
        getLogLog().warn("You have tried to set a null error-handler.");
        return;
    }
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex )
        this->errorHandler = eh;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}


void 
Appender::setLayout(std::auto_ptr<Layout> lo)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex )
        this->layout = lo;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}


void 
Appender::setName(const std::string& name)
{
    this->name = name;
}


void 
Appender::setThreshold(const Priority* th)
{
    delete this->threshold;
    this->threshold = NULL; // If th is NULL, we NULL out the
                            // threshold value in this instance

    if(th != NULL) {
        this->threshold = new Priority( *th );
    }
}


