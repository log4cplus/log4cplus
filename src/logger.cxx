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
 : value(val.get())
{
    init();
}


Logger::Logger(spi::LoggerImpl *ptr)
 : value(ptr)
{
    init();
}


Logger::Logger(const Logger& rhs)
  : spi::AppenderAttachable (rhs)
  , value(rhs.value)
{
    init();
}


Logger&
Logger::operator=(const Logger& rhs)
{
    if (value != rhs.value) {
        spi::LoggerImpl *oldValue = value;

        value = rhs.value;
        init();
        if(oldValue != NULL) {
            oldValue->removeReference();
        }
    }

    return *this;
}


Logger::~Logger() 
{
    if (value != NULL) {
        value->removeReference();
    }
}



//////////////////////////////////////////////////////////////////////////////
// Logger Methods
//////////////////////////////////////////////////////////////////////////////

Logger
Logger::getParent() {
    validate(__FILE__, __LINE__);
    if(value->parent.get() != NULL) {
        return Logger(value->parent);
    }
    else {
        value->getLogLog().error(LOG4CPLUS_TEXT("********* This logger has no parent: " + getName()));
        return *this;
    }
}


void
Logger::init() {
    if(value == NULL) return;
    value->addReference();
}


void
Logger::validate(const char *file, int line) const
{
    if(value == NULL) {
        SharedObjectPtr<LogLog> loglog = LogLog::getLogLog();
        loglog->error(LOG4CPLUS_TEXT("Logger::validate()- Internal log4cplus error: NullPointerException"));
        log4cplus::helpers::throwNullPointerException(file, line);
    }
}



void 
Logger::callAppenders(const spi::InternalLoggingEvent& event)
{
    validate(__FILE__, __LINE__);
    value->callAppenders(event);
}


void 
Logger::closeNestedAppenders()
{
    validate(__FILE__, __LINE__);
    value->closeNestedAppenders();
}


bool 
Logger::isEnabledFor(LogLevel ll) const
{
    validate(__FILE__, __LINE__);
    return value->isEnabledFor(ll);
}


void 
Logger::log(LogLevel ll, const log4cplus::tstring& message,
            const char* file, int line)
{
    validate(__FILE__, __LINE__);
    value->log(ll, message, file, line);
}


LogLevel
Logger::getChainedLogLevel() const
{
    validate(__FILE__, __LINE__);
    return value->getChainedLogLevel();
}


LogLevel
Logger::getLogLevel() const
{
    validate(__FILE__, __LINE__);
    return value->getLogLevel();
}


void 
Logger::setLogLevel(LogLevel ll)
{
    validate(__FILE__, __LINE__);
    value->setLogLevel(ll);
}


Hierarchy& 
Logger::getHierarchy() const
{ 
    validate(__FILE__, __LINE__);
    return value->getHierarchy();
}


log4cplus::tstring 
Logger::getName() const
{
    validate(__FILE__, __LINE__);
    return value->getName();
}


bool 
Logger::getAdditivity() const
{
    validate(__FILE__, __LINE__);
    return value->getAdditivity();
}


void 
Logger::setAdditivity(bool additive) 
{ 
    validate(__FILE__, __LINE__);
    value->setAdditivity(additive);
}


void 
Logger::addAppender(SharedAppenderPtr newAppender)
{
    validate(__FILE__, __LINE__);
    value->addAppender(newAppender);
}


SharedAppenderPtrList 
Logger::getAllAppenders()
{
    validate(__FILE__, __LINE__);
    return value->getAllAppenders();
}


SharedAppenderPtr 
Logger::getAppender(const log4cplus::tstring& name)
{
    validate(__FILE__, __LINE__);
    return value->getAppender(name);
}


void 
Logger::removeAllAppenders()
{
    validate(__FILE__, __LINE__);
    value->removeAllAppenders();
}


void 
Logger::removeAppender(SharedAppenderPtr appender)
{
    validate(__FILE__, __LINE__);
    value->removeAppender(appender);
}


void 
Logger::removeAppender(const log4cplus::tstring& name)
{
    validate(__FILE__, __LINE__);
    value->removeAppender(name);
}


void 
Logger::forcedLog(LogLevel ll, const log4cplus::tstring& message,
                    const char* file, int line)
{
    validate(__FILE__, __LINE__);
    value->forcedLog(ll, message, file, line);
}


