// Module:  Log4CPLUS
// File:    appenderattachableimpl.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright (C) The Apache Software Foundation. All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

#include <log4cplus/appender.h>
#include <log4cplus/helpers/appenderattachableimpl.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggingevent.h>
#include <algorithm>

using namespace log4cplus;
using namespace log4cplus::helpers;


AppenderAttachableImpl::AppenderAttachableImpl()
 : mutex(LOG4CPLUS_MUTEX_CREATE)
{
}


AppenderAttachableImpl::~AppenderAttachableImpl()
{
   LOG4CPLUS_MUTEX_FREE( mutex );
}


void
AppenderAttachableImpl::addAppender(SharedAppenderPtr newAppender)
LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
    if(newAppender == NULL) {
        getLogLog().warn("Tried to add NULL appender");
        return;
    }

    ListType::iterator it = 
        std::find(appenderList.begin(), appenderList.end(), newAppender);
    if(it == appenderList.end()) {
        appenderList.push_back(newAppender);
    }
LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX


AppenderAttachableImpl::ListType
AppenderAttachableImpl::getAllAppenders()
LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
    return appenderList;
LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX


SharedAppenderPtr 
AppenderAttachableImpl::getAppender(const std::string& name)
LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
    for(ListType::iterator it=appenderList.begin(); 
        it!=appenderList.end(); 
        ++it)
    {
        if((*it)->getName() == name) {
            return *it;
        }
    }

    return SharedAppenderPtr(NULL);
LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX


void 
AppenderAttachableImpl::removeAllAppenders()
{
    log4cplus::thread::Guard guard(mutex);
    appenderList.erase(appenderList.begin(), appenderList.end());
}


void 
AppenderAttachableImpl::removeAppender(SharedAppenderPtr appender)
LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
    ListType::iterator it =
        std::find(appenderList.begin(), appenderList.end(), appender);
    if(it != appenderList.end()) {
        appenderList.erase(it);
    }
LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX


void 
AppenderAttachableImpl::removeAppender(const std::string& name)
{
    removeAppender(getAppender(name));
}


int 
AppenderAttachableImpl::appendLoopOnAppenders(const spi::InternalLoggingEvent& event) const
{
    int count = 0;

    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        for(ListType::const_iterator it=appenderList.begin();
            it!=appenderList.end();
            ++it)
        {
            ++count;
            (*it)->doAppend(event);
        }
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX

    return count;
}


