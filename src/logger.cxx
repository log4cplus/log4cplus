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


namespace log4cplus
{


Logger 
DefaultLoggerFactory::makeNewLoggerInstance (const log4cplus::tstring & name,
    Hierarchy& h)
{ 
    return Logger (new spi::LoggerImpl (name, h));
}


//////////////////////////////////////////////////////////////////////////////
// static Logger Methods
//////////////////////////////////////////////////////////////////////////////
//
Hierarchy & 
Logger::getDefaultHierarchy ()
{
    static Hierarchy defaultHierarchy;

    return defaultHierarchy;
}


bool 
Logger::exists (const log4cplus::tstring & name) 
{
    return getDefaultHierarchy().exists(name); 
}


LoggerList
Logger::getCurrentLoggers () 
{
    return getDefaultHierarchy ().getCurrentLoggers ();
}


Logger 
Logger::getInstance (const log4cplus::tstring& name) 
{ 
    return getDefaultHierarchy().getInstance(name); 
}


Logger 
Logger::getInstance (const log4cplus::tstring& name,
    spi::LoggerFactory& factory)
{ 
    return getDefaultHierarchy().getInstance(name, factory); 
}


Logger 
Logger::getRoot () 
{ 
    return getDefaultHierarchy ().getRoot (); 
}


void 
Logger::shutdown () 
{ 
    getDefaultHierarchy ().shutdown (); 
}



//////////////////////////////////////////////////////////////////////////////
// Logger ctors and dtor
//////////////////////////////////////////////////////////////////////////////

Logger::Logger ()
    : value (0)
{ }


Logger::Logger (spi::LoggerImpl * ptr)
    : value (ptr)
{
    if (value)
        value->addReference ();
}


Logger::Logger (const Logger& rhs)
    : spi::AppenderAttachable (rhs)
    , value (rhs.value)
{
    if (value)
        value->addReference ();
}


Logger &
Logger::operator = (const Logger& rhs)
{
    Logger (rhs).swap (*this);
    return *this;
}


Logger::~Logger () 
{
    if (value)
        value->removeReference ();
}


//////////////////////////////////////////////////////////////////////////////
// Logger Methods
//////////////////////////////////////////////////////////////////////////////

void
Logger::swap (Logger & other)
{
    std::swap (value, other.value);
}


Logger
Logger::getParent () const 
{
    if (value->parent)
        return Logger (value->parent.get ());
    else
    {
        value->getLogLog().error(LOG4CPLUS_TEXT("********* This logger has no parent: " + getName()));
        return *this;
    }
}


void 
Logger::addAppender (SharedAppenderPtr newAppender)
{
    value->addAppender(newAppender);
}


SharedAppenderPtrList 
Logger::getAllAppenders ()
{
    return value->getAllAppenders();
}


SharedAppenderPtr 
Logger::getAppender (const log4cplus::tstring& name)
{
    return value->getAppender (name);
}


void 
Logger::removeAllAppenders ()
{
    value->removeAllAppenders ();
}


void 
Logger::removeAppender (SharedAppenderPtr appender)
{
    value->removeAppender(appender);
}


void 
Logger::removeAppender (const log4cplus::tstring& name)
{
    value->removeAppender (name);
}


void
Logger::assertion (bool assertionVal, const log4cplus::tstring& msg) const
{
    if (! assertionVal)
        log (FATAL_LOG_LEVEL, msg);
}


void
Logger::closeNestedAppenders () const
{
    value->closeNestedAppenders ();
}


bool
Logger::isEnabledFor (LogLevel ll) const
{
    return value->isEnabledFor (ll);
}


void
Logger::log (LogLevel ll, const log4cplus::tstring& message, const char* file,
    int line) const
{
    value->log (ll, message, file, line);
}


void
Logger::forcedLog (LogLevel ll, const log4cplus::tstring& message,
    const char* file, int line) const
{
    value->forcedLog (ll, message, file, line);
}


void
Logger::callAppenders (const spi::InternalLoggingEvent& event) const
{
    value->callAppenders (event);
}


LogLevel
Logger::getChainedLogLevel () const
{
    return value->getChainedLogLevel ();
}


LogLevel
Logger::getLogLevel() const
{
    return value->getLogLevel ();
}


void
Logger::setLogLevel (LogLevel ll)
{
    value->setLogLevel (ll);
}


Hierarchy &
Logger::getHierarchy () const
{ 
    return value->getHierarchy ();
}


log4cplus::tstring const &
Logger::getName () const
{
    return value->getName ();
}


bool
Logger::getAdditivity () const
{
    return value->getAdditivity ();
}


void
Logger::setAdditivity (bool additive)
{ 
    value->setAdditivity (additive);
}


} // namespace log4cplus
