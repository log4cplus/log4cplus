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
// Revision 1.8  2003/05/14 23:07:16  tcsmith
// Fixed some TABs.
//
// Revision 1.7  2003/04/18 21:00:34  tcsmith
// Converted from std::string to log4cplus::tstring.
//
// Revision 1.6  2003/04/07 16:35:21  tcsmith
// Fixed gcc 3.2 compilation error.
//
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
#include <log4cplus/helpers/stringhelper.h>
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
OnlyOnceErrorHandler::error(const log4cplus::tstring& err)
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
   name( LOG4CPLUS_TEXT("") ),
   threshold(NOT_SET_LOG_LEVEL),
   errorHandler(new OnlyOnceErrorHandler()),
   closed(false)
{
}



Appender::Appender(log4cplus::helpers::Properties properties)
 : layout(new SimpleLayout()),
   name( LOG4CPLUS_TEXT("") ),
   threshold(NOT_SET_LOG_LEVEL),
   errorHandler(new OnlyOnceErrorHandler()),
   closed(false)
{
    if(properties.exists( LOG4CPLUS_TEXT("layout") )) {
       log4cplus::tstring factoryName = properties.getProperty( LOG4CPLUS_TEXT("layout") );
        LayoutFactory* factory = getLayoutFactoryRegistry().get(factoryName);
        if(factory == 0) {
            getLogLog().error(  LOG4CPLUS_TEXT("Cannot find LayoutFactory: \"")
                              + factoryName
                              + LOG4CPLUS_TEXT("\"") );
            return;
        }

        Properties layoutProperties =
                properties.getPropertySubset( LOG4CPLUS_TEXT("layout.") );
        try {
            std::auto_ptr<Layout> newLayout = factory->createObject(layoutProperties);
            if(newLayout.get() == 0) {
                getLogLog().error(  LOG4CPLUS_TEXT("Failed to create appender: ")
                                  + factoryName);
            }
            else {
                layout = newLayout;
            }
        }
        catch(std::exception& e) {
            getLogLog().error(  LOG4CPLUS_TEXT("Error while creating Layout: ")
                              + LOG4CPLUS_C_STR_TO_TSTRING(e.what()));
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

    getLogLog().debug(  LOG4CPLUS_TEXT("Destroying appender named [")
                      + name
                      + LOG4CPLUS_TEXT("]."));
    close();
    closed = true;
}



void
Appender::doAppend(const log4cplus::spi::InternalLoggingEvent& event)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex )
        if(closed) {
            getLogLog().error(  LOG4CPLUS_TEXT("Attempted to append to closed appender named [")
                              + name
                              + LOG4CPLUS_TEXT("]."));
            return;
        }

        if(!isAsSevereAsThreshold(event.ll)) {
            return;
        }

	if(checkFilter(filter.get(), event) == DENY) {
            return;
	}

        append(event);
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}



log4cplus::tstring
Appender::getName()
{
    return name;
}



void
Appender::setName(const log4cplus::tstring& name)
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
        getLogLog().warn(LOG4CPLUS_TEXT("You have tried to set a null error-handler."));
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


