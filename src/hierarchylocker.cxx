// Module:  Log4CPLUS
// File:    hierarchylocker.cxx
// Created: 8/2003
// Author:  Tad E. Smith
//
//
// Copyright 2003-2009 Tad E. Smith
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


