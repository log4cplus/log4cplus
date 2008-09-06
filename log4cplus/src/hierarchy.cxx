// Module:  Log4CPLUS
// File:    hierarchy.cxx
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

#include <log4cplus/hierarchy.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggerimpl.h>
#include <log4cplus/spi/rootlogger.h>
#include <utility>

using namespace log4cplus;
using namespace log4cplus::helpers;


//////////////////////////////////////////////////////////////////////////////
// File "Local" methods
//////////////////////////////////////////////////////////////////////////////

namespace {
    static
    bool startsWith(log4cplus::tstring teststr, log4cplus::tstring substr) {
        bool val = false;
        if(teststr.length() > substr.length()) {
            val =  teststr.substr(0, substr.length()) == substr;
        }

        return val;
    }
}



//////////////////////////////////////////////////////////////////////////////
// log4cplus::Hierarchy static declarations
//////////////////////////////////////////////////////////////////////////////

const LogLevel log4cplus::Hierarchy::DISABLE_OFF = -1;
const LogLevel log4cplus::Hierarchy::DISABLE_OVERRIDE = -2;



//////////////////////////////////////////////////////////////////////////////
// log4cplus::Hierarchy ctor and dtor
//////////////////////////////////////////////////////////////////////////////

Hierarchy::Hierarchy()
  : hashtable_mutex(LOG4CPLUS_MUTEX_CREATE),
    defaultFactory(new DefaultLoggerFactory()),
    root(NULL),
    disableValue(DISABLE_OFF),  // Don't disable any LogLevel level by default.
    emittedNoAppenderWarning(false),
    emittedNoResourceBundleWarning(false)
{
    root = Logger( new spi::RootLogger(*this, DEBUG_LOG_LEVEL) );
}


Hierarchy::~Hierarchy()
{
    shutdown();
    LOG4CPLUS_MUTEX_FREE( hashtable_mutex );
}



//////////////////////////////////////////////////////////////////////////////
// log4cplus::Hierarchy public methods
//////////////////////////////////////////////////////////////////////////////

void 
Hierarchy::clear() 
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( hashtable_mutex )
        provisionNodes.erase(provisionNodes.begin(), provisionNodes.end());
        loggerPtrs.erase(loggerPtrs.begin(), loggerPtrs.end());
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
}


bool
Hierarchy::exists(const log4cplus::tstring& name)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( hashtable_mutex )
        LoggerMap::iterator it = loggerPtrs.find(name);
        return it != loggerPtrs.end();
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
}


void 
Hierarchy::disable(const log4cplus::tstring& loglevelStr)
{
    if(disableValue != DISABLE_OVERRIDE) {
        disableValue = getLogLevelManager().fromString(loglevelStr);
    }
}


void 
Hierarchy::disable(LogLevel ll) 
{
    if(disableValue != DISABLE_OVERRIDE) {
        disableValue = ll;
    }
}


void 
Hierarchy::disableAll() 
{ 
    disable(FATAL_LOG_LEVEL);
}


void 
Hierarchy::disableDebug() 
{ 
    disable(DEBUG_LOG_LEVEL);
}


void 
Hierarchy::disableInfo() 
{ 
    disable(INFO_LOG_LEVEL);
}


void 
Hierarchy::enableAll() 
{ 
    disableValue = DISABLE_OFF; 
}


Logger 
Hierarchy::getInstance(const log4cplus::tstring& name) 
{ 
    return getInstance(name, *defaultFactory); 
}


Logger 
Hierarchy::getInstance(const log4cplus::tstring& name, spi::LoggerFactory& factory)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( hashtable_mutex )
        return getInstanceImpl(name, factory);
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
}


LoggerList 
Hierarchy::getCurrentLoggers()
{
    LoggerList ret;
    
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( hashtable_mutex )
        initializeLoggerList(ret);
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;

    return ret;
}


bool 
Hierarchy::isDisabled(int level) 
{ 
    return disableValue >= level; 
}


Logger 
Hierarchy::getRoot() const
{ 
    return root; 
}


void 
Hierarchy::resetConfiguration()
{
    getRoot().setLogLevel(DEBUG_LOG_LEVEL);
    disableValue = DISABLE_OFF;

    shutdown();

    LoggerList loggers = getCurrentLoggers();
    LoggerList::iterator it = loggers.begin();
    while(it != loggers.end()) {
        (*it).setLogLevel(NOT_SET_LOG_LEVEL);
        (*it).setAdditivity(true);
        ++it;
    }

}


void 
Hierarchy::setLoggerFactory(std::auto_ptr<spi::LoggerFactory> factory) 
{ 
    defaultFactory = factory; 
}


void 
Hierarchy::shutdown()
{
    LoggerList loggers = getCurrentLoggers();

    // begin by closing nested appenders
    // then, remove all appenders
    root.closeNestedAppenders();
    root.removeAllAppenders();

    // repeat
    LoggerList::iterator it = loggers.begin();
    while(it != loggers.end()) {
        (*it).closeNestedAppenders();
        (*it).removeAllAppenders();
        ++it;
    }
}



//////////////////////////////////////////////////////////////////////////////
// log4cplus::Hierarchy private methods
//////////////////////////////////////////////////////////////////////////////

Logger 
Hierarchy::getInstanceImpl(const log4cplus::tstring& name, spi::LoggerFactory& factory)
{
     LoggerMap::iterator it = loggerPtrs.find(name);
     if(it != loggerPtrs.end()) {
         return (*it).second;
     }
     else {
         // Need to create a new logger
         Logger logger = factory.makeNewLoggerInstance(name, *this);
         bool inserted = loggerPtrs.insert(std::make_pair(name, logger)).second;
         if(!inserted) {
             getLogLog().error(LOG4CPLUS_TEXT("Hierarchy::getInstanceImpl()- Insert failed"));
             throw std::runtime_error("Hierarchy::getInstanceImpl()- Insert failed");
         }
         
         ProvisionNodeMap::iterator it2 = provisionNodes.find(name);
         if(it2 != provisionNodes.end()) {
             updateChildren(it2->second, logger);
             bool deleted = (provisionNodes.erase(name) > 0);
             if(!deleted) {
                 getLogLog().error(LOG4CPLUS_TEXT("Hierarchy::getInstanceImpl()- Delete failed"));
                 throw std::runtime_error("Hierarchy::getInstanceImpl()- Delete failed");
             }
         }
         updateParents(logger);
         
         return logger;
     }
}


void 
Hierarchy::initializeLoggerList(LoggerList& list) const
{
    for(LoggerMap::const_iterator it=loggerPtrs.begin(); 
        it!= loggerPtrs.end(); 
        ++it) 
    {
        list.push_back((*it).second);
    }
}


void 
Hierarchy::updateParents(Logger logger)
{
    log4cplus::tstring name = logger.getName();
    size_t length = name.length();
    bool parentFound = false;

    // if name = "w.x.y.z", loop thourgh "w.x.y", "w.x" and "w", but not "w.x.y.z"
    for(size_t i=name.find_last_of(LOG4CPLUS_TEXT('.'), length-1); 
        i != log4cplus::tstring::npos; 
        i = name.find_last_of(LOG4CPLUS_TEXT('.'), i-1)) 
    {
        log4cplus::tstring substr = name.substr(0, i);

        LoggerMap::iterator it = loggerPtrs.find(substr);
        if(it != loggerPtrs.end()) {
            parentFound = true;
            logger.value->parent = it->second.value;
            break;  // no need to update the ancestors of the closest ancestor
        }
        else {
            ProvisionNodeMap::iterator it2 = provisionNodes.find(substr);
            if(it2 != provisionNodes.end()) {
                it2->second.push_back(logger);
            }
            else {
                ProvisionNode node;
                node.push_back(logger);
                std::pair<ProvisionNodeMap::iterator, bool> tmp = 
                    provisionNodes.insert(std::make_pair(substr, node));
                //bool inserted = provisionNodes.insert(std::make_pair(substr, node)).second;
                if(!tmp.second) {
                    getLogLog().error(LOG4CPLUS_TEXT("Hierarchy::updateParents()- Insert failed"));
                    throw std::runtime_error("Hierarchy::updateParents()- Insert failed");
                }
            }
        } // end if Logger found
    } // end for loop

    if(!parentFound) {
        logger.value->parent = root.value;
    }
}


void 
Hierarchy::updateChildren(ProvisionNode& pn, Logger logger)
{

    for(ProvisionNode::iterator it=pn.begin(); it!=pn.end(); ++it) {
        Logger& c = *it;
        // Unless this child already points to a correct (lower) parent,
        // make logger.parent point to c.parent and c.parent to logger.
        if( !startsWith(c.value->parent->getName(), logger.getName()) ) {
            logger.value->parent = c.value->parent;
            c.value->parent = logger.value;
        }
    }
}

