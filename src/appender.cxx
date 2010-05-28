// Module:  Log4CPLUS
// File:    appender.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright 2003-2010 Tad E. Smith
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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



void
OnlyOnceErrorHandler::reset()
{
    firstTime = true;
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



Appender::Appender(const log4cplus::helpers::Properties properties)
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
    tstring filterName, factoryName;
    while( filterProps.exists(filterName = convertIntegerToString(++filterCount)) ) {
        factoryName = filterProps.getProperty(filterName);
        FilterFactory* factory = getFilterFactoryRegistry().get(factoryName);

        if(factory == 0) {
            tstring err = LOG4CPLUS_TEXT("Appender::ctor()- Cannot find FilterFactory: ");
            getLogLog().error(err + factoryName);
            continue;
        }
        FilterPtr tmp_filter = factory->createObject
                      (filterProps.getPropertySubset(filterName + LOG4CPLUS_TEXT(".")));
        if(tmp_filter.get() == 0) {
            tstring err = LOG4CPLUS_TEXT("Appender::ctor()- Failed to create filter: ");
            getLogLog().error(err + filterName);
        }
        if(filterChain.get() == 0) {
            filterChain = tmp_filter;
        }
        else {
            filterChain->appendFilter(tmp_filter);
        }
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
    if(closed) {
        return;
    }

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

        if(!isAsSevereAsThreshold(event.getLogLevel())) {
            return;
        }

        if(checkFilter(filter.get(), event) == DENY) {
            return;
        }

        append(event);
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
}



log4cplus::tstring
Appender::getName()
{
    return name;
}



void
Appender::setName(const log4cplus::tstring& name_)
{
    this->name = name_;
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


