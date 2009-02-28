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

#include <log4cplus/appender.h>
#include <log4cplus/layout.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/pointer.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/spi/factory.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/internal/internal.h>


namespace log4cplus
{

using namespace log4cplus::helpers;
using namespace log4cplus::spi;


template class helpers::SharedObjectPtr<Appender>;


///////////////////////////////////////////////////////////////////////////////
// log4cplus::ErrorHandler dtor
///////////////////////////////////////////////////////////////////////////////

ErrorHandler::ErrorHandler ()
{ }


ErrorHandler::~ErrorHandler()
{ }



///////////////////////////////////////////////////////////////////////////////
// log4cplus::OnlyOnceErrorHandler 
///////////////////////////////////////////////////////////////////////////////

OnlyOnceErrorHandler::OnlyOnceErrorHandler()
    : firstTime(true)
{ }


OnlyOnceErrorHandler::~OnlyOnceErrorHandler ()
{ }


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
   errorHandler(new OnlyOnceErrorHandler),
   closed(false)
{
}



Appender::Appender(const log4cplus::helpers::Properties & properties)
    : layout(new SimpleLayout())
    , name()
    , threshold(NOT_SET_LOG_LEVEL)
    , errorHandler(new OnlyOnceErrorHandler)
    , closed(false)
{
    if(properties.exists( LOG4CPLUS_TEXT("layout") ))
    {
        log4cplus::tstring const & factoryName
            = properties.getProperty( LOG4CPLUS_TEXT("layout") );
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
            std::auto_ptr<Layout> newLayout(factory->createObject(layoutProperties));
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

    // Support for appender.Threshold in properties configuration file
    if(properties.exists(LOG4CPLUS_TEXT("Threshold"))) {
        tstring tmp = properties.getProperty(LOG4CPLUS_TEXT("Threshold"));
        tmp = log4cplus::helpers::toUpper(tmp);
        threshold = log4cplus::getLogLevelManager().fromString(tmp);
    }

    // Configure the filters
    Properties filterProps = properties.getPropertySubset( LOG4CPLUS_TEXT("filters.") );
    int filterCount = 0;
    FilterPtr filterChain;
    tstring filterName;
    while (filterProps.exists(
        filterName = convertIntegerToString (++filterCount)))
    {
        tstring const & factoryName = filterProps.getProperty(filterName);
        FilterFactory* factory = getFilterFactoryRegistry().get(factoryName);

        if(! factory)
        {
            tstring err = LOG4CPLUS_TEXT("Appender::ctor()- Cannot find FilterFactory: ");
            getLogLog().error(err + factoryName);
            continue;
        }
        FilterPtr tmpFilter = factory->createObject (
            filterProps.getPropertySubset(filterName + LOG4CPLUS_TEXT(".")));
        if (! tmpFilter)
        {
            tstring err = LOG4CPLUS_TEXT("Appender::ctor()- Failed to create filter: ");
            getLogLog().error(err + filterName);
        }
        if (! filterChain)
            filterChain = tmpFilter;
        else
            filterChain->appendFilter(tmpFilter);
    }
    setFilter(filterChain);
}


Appender::~Appender()
{ }



///////////////////////////////////////////////////////////////////////////////
// log4cplus::Appender public methods
///////////////////////////////////////////////////////////////////////////////

void
Appender::destructorImpl()
{
    getLogLog().debug(  LOG4CPLUS_TEXT("Destroying appender named [")
                      + name
                      + LOG4CPLUS_TEXT("]."));

    // An appender might be closed then destroyed. There is no
    // point in closing twice.
    if(closed)
        return;

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

        if (! isAsSevereAsThreshold(event.getLogLevel()))
            return;

        if (checkFilter(filter.get(), event) == DENY)
            return;

        append(event);
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
}


tstring &
Appender::formatEvent (const spi::InternalLoggingEvent& event) const
{
    internal::appender_sratch_pad & appender_sp = internal::get_appender_sp ();
    detail::clear_tostringstream (appender_sp.oss);
    layout->formatAndAppend(appender_sp.oss, event);
    appender_sp.str = appender_sp.oss.str();
    return appender_sp.str;
}


log4cplus::tstring
Appender::getName()
{
    return name;
}



void
Appender::setName(const log4cplus::tstring& n)
{
    this->name = n;
}


ErrorHandler*
Appender::getErrorHandler()
{
    return errorHandler.get();
}



void
Appender::setErrorHandler(std::auto_ptr<ErrorHandler> eh)
{
    if (! eh.get())
    {
        // We do not throw exception here since the cause is probably a
        // bad config file.
        getLogLog().warn(LOG4CPLUS_TEXT("You have tried to set a null error-handler."));
        return;
    }
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex )
        this->errorHandler = eh;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
}



void
Appender::setLayout(std::auto_ptr<Layout> lo)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex )
        this->layout = lo;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
}



Layout*
Appender::getLayout()
{
    return layout.get();
}


} // namespace log4cplus
