// Module:  Log4CPLUS
// File:    appender.cxx
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
// Revision 1.5  2003/04/03 01:37:16  tcsmith
// Removed tabs from this file.
//
// Revision 1.4  2003/04/03 00:35:22  tcsmith
// Standardized the formatting.
//

#include <log4cplus/appender.h>
#include <log4cplus/layout.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/pointer.h>
#include <log4cplus/spi/factory.h>
#include <log4cplus/spi/loggingevent.h>

using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::spi;


///////////////////////////////////////////////////////////////////////////////
// log4cplus::ErrorHandler dtor
///////////////////////////////////////////////////////////////////////////////

ErrorHandler::~ErrorHandler()
{
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::OnlyOnceErrorHandler public methods
///////////////////////////////////////////////////////////////////////////////

void
OnlyOnceErrorHandler::error(const std::string& err) 
{
    if(firstTime) {
        getLogLog().error(err);
        firstTime = false;
    }
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::Appender ctors
///////////////////////////////////////////////////////////////////////////////

Appender::Appender()
 : layout(new SimpleLayout()),
   name(""),
   threshold(NOT_SET_LOG_LEVEL),
   errorHandler(new OnlyOnceErrorHandler()),
   closed(false)
{
}



Appender::Appender(log4cplus::helpers::Properties properties)
 : layout(new SimpleLayout()),
   name(""),
   threshold(NOT_SET_LOG_LEVEL),
   errorHandler(new OnlyOnceErrorHandler()),
   closed(false)
{
    if(properties.exists("layout")) {
        std::string factoryName = properties.getProperty("layout");
        LayoutFactory* factory = getLayoutFactoryRegistry().get(factoryName);
        if(factory == 0) {
            getLogLog().error("Cannot find LayoutFactory: \"" + factoryName + "\"");
            return;
        }

        Properties layoutProperties = properties.getPropertySubset("layout.");
        try {
            std::auto_ptr<Layout> newLayout = factory->createObject(layoutProperties);
            if(newLayout.get() == 0) {
                getLogLog().error("Failed to create appender: " + factoryName);
            }
            else {
                layout = newLayout;
            }
        }
        catch(std::exception& e) {
            getLogLog().error("Error while creating Layout: " + std::string(e.what()));
            return;
        }

    }
}


///////////////////////////////////////////////////////////////////////////////
// log4cplus::Appender public methods
///////////////////////////////////////////////////////////////////////////////

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



void 
Appender::doAppend(const log4cplus::spi::InternalLoggingEvent& event)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex )
        if(closed) {
            getLogLog().error("Attempted to append to closed appender named ["+name+"].");
            return;
        }

        if(!isAsSevereAsThreshold(event.ll)) {
            return;
        }

        append(event);
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}



std::string 
Appender::getName()
{
    return name;
}



void 
Appender::setName(const std::string& name)
{
    this->name = name;
}


ErrorHandler* 
Appender::getErrorHandler()
{
    return errorHandler.get();
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



Layout* 
Appender::getLayout()
{
    return layout.get();
}


