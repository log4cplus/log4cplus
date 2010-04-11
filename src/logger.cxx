// Module:  Log4CPLUS
// File:    logger.cxx
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

#include <log4cplus/logger.h>
#include <log4cplus/appender.h>
#include <log4cplus/hierarchy.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggerimpl.h>
#include <stdexcept>

using namespace log4cplus;
using namespace log4cplus::helpers;


Logger 
DefaultLoggerFactory::makeNewLoggerInstance(const log4cplus::tstring& name, Hierarchy& h)
{ 
    return Logger( new spi::LoggerImpl(name, h) );
}


//////////////////////////////////////////////////////////////////////////////
// static Logger Methods
//////////////////////////////////////////////////////////////////////////////
//
Hierarchy& 
Logger::getDefaultHierarchy()
{
    static Hierarchy defaultHierarchy;

    return defaultHierarchy;
}


bool 
Logger::exists(const log4cplus::tstring& name) 
{
    return getDefaultHierarchy().exists(name); 
}


LoggerList
Logger::getCurrentLoggers() 
{
    return getDefaultHierarchy().getCurrentLoggers();
}


Logger 
Logger::getInstance(const log4cplus::tstring& name) 
{ 
    return getDefaultHierarchy().getInstance(name); 
}


Logger 
Logger::getInstance(const log4cplus::tstring& name, spi::LoggerFactory& factory)
{ 
    return getDefaultHierarchy().getInstance(name, factory); 
}


Logger 
Logger::getRoot() 
{ 
    return getDefaultHierarchy().getRoot(); 
}


void 
Logger::shutdown() 
{ 
    getDefaultHierarchy().shutdown(); 
}



//////////////////////////////////////////////////////////////////////////////
// Logger ctors and dtor
//////////////////////////////////////////////////////////////////////////////

Logger::Logger(const spi::SharedLoggerImplPtr& val)
    : value(val)
{ }


Logger::Logger(spi::LoggerImpl *ptr)
    : value(ptr)
{ }


Logger::Logger(const Logger& rhs)
    : spi::AppenderAttachable (rhs)
    , value(rhs.value)
{ }


Logger&
Logger::operator=(const Logger& rhs)
{
    value = rhs.value;
    return *this;
}


Logger::~Logger() 
{ }



//////////////////////////////////////////////////////////////////////////////
// Logger Methods
//////////////////////////////////////////////////////////////////////////////

Logger
Logger::getParent() const 
{
    if (value->parent)
        return Logger(value->parent);
    else
    {
        value->getLogLog().error(LOG4CPLUS_TEXT("********* This logger has no parent: " + getName()));
        return *this;
    }
}


void 
Logger::addAppender(SharedAppenderPtr newAppender)
{
    value->addAppender(newAppender);
}


SharedAppenderPtrList 
Logger::getAllAppenders()
{
    return value->getAllAppenders();
}


SharedAppenderPtr 
Logger::getAppender(const log4cplus::tstring& name)
{
    return value->getAppender(name);
}


void 
Logger::removeAllAppenders()
{
    value->removeAllAppenders();
}


void 
Logger::removeAppender(SharedAppenderPtr appender)
{
    value->removeAppender(appender);
}


void 
Logger::removeAppender(const log4cplus::tstring& name)
{
    value->removeAppender(name);
}
