// Module:  Log4CPLUS
// File:    hierarchylocker.cxx
// Created: 8/2003
// Author:  Tad E. Smith
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

#include <log4cplus/hierarchylocker.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggerimpl.h>


using namespace log4cplus;
using namespace log4cplus::helpers;



//////////////////////////////////////////////////////////////////////////////
// log4cplus::HierarchyLocker ctor and dtor
//////////////////////////////////////////////////////////////////////////////

HierarchyLocker::HierarchyLocker(Hierarchy& _h)
: h(_h),
  hierarchyLocker(h.hashtable_mutex),
  loggerList()
{
    // Get a copy of all of the Hierarchy's Loggers (except the Root Logger)
    h.initializeLoggerList(loggerList);

    // Lock all of the Hierarchy's Loggers' mutexs
    try {
        for(LoggerList::iterator it=loggerList.begin(); it!=loggerList.end(); ++it) {
            LOG4CPLUS_MUTEX_LOCK( (*it).value->appender_list_mutex ) ;
        }
    }
    catch(...) {
        h.getLogLog().error(LOG4CPLUS_TEXT("HierarchyLocker::ctor()- An error occurred while locking"));
        // TODO --> We need to unlock any Logger mutex that we were able to lock
        throw;
    }
}
 

HierarchyLocker::~HierarchyLocker()
{
    try {
        for(LoggerList::iterator it=loggerList.begin(); it!=loggerList.end(); ++it) {
            LOG4CPLUS_MUTEX_UNLOCK( (*it).value->appender_list_mutex ) ;
        }
    }
    catch(...) {
        h.getLogLog().error(LOG4CPLUS_TEXT("HierarchyLocker::dtor()- An error occurred while unlocking"));
        throw;
    }
}

void 
HierarchyLocker::resetConfiguration()
{
    Logger root = h.getRoot();
    h.disable(log4cplus::Hierarchy::DISABLE_OFF);

    // begin by closing nested appenders
    // then, remove all appenders
    root.setLogLevel(DEBUG_LOG_LEVEL);
    root.closeNestedAppenders();
    root.removeAllAppenders();

    // repeat
    for(LoggerList::iterator it=loggerList.begin(); it!=loggerList.end(); ++it) {
        LOG4CPLUS_MUTEX_UNLOCK( (*it).value->appender_list_mutex ) ;
        (*it).closeNestedAppenders();
        (*it).removeAllAppenders();
        LOG4CPLUS_MUTEX_LOCK( (*it).value->appender_list_mutex ) ;
        (*it).setLogLevel(NOT_SET_LOG_LEVEL);
        (*it).setAdditivity(true);
    }
}


Logger 
HierarchyLocker::getInstance(const log4cplus::tstring& name)
{
    return h.getInstanceImpl(name, *h.getLoggerFactory());
}


Logger 
HierarchyLocker::getInstance(const log4cplus::tstring& name, spi::LoggerFactory& factory)
{
    return h.getInstanceImpl(name, factory);
}


void 
HierarchyLocker::addAppender(Logger& logger, log4cplus::SharedAppenderPtr& appender)
{
    for(LoggerList::iterator it=loggerList.begin(); it!=loggerList.end(); ++it) {
        if((*it).value == logger.value) {
            LOG4CPLUS_MUTEX_UNLOCK( logger.value->appender_list_mutex );
            logger.addAppender(appender);
            LOG4CPLUS_MUTEX_LOCK( logger.value->appender_list_mutex );
            return;
        }
    }
    
    // I don't have this Logger locked
    logger.addAppender(appender);
}


