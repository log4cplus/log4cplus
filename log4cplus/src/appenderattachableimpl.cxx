// Module:  Log4CPLUS
// File:    appenderattachableimpl.cxx
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
// Revision 1.5  2003/05/14 23:06:40  tcsmith
// Corrected removeAllAppenders() to used the synchronization macros instead
// of a creating a Guard on the mutex.
//
// Revision 1.4  2003/04/18 21:00:35  tcsmith
// Converted from std::string to log4cplus::tstring.
//
// Revision 1.3  2003/04/03 00:27:41  tcsmith
// Standardized the formatting.
//

#include <log4cplus/appender.h>
#include <log4cplus/helpers/appenderattachableimpl.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggingevent.h>

#include <algorithm>

using namespace log4cplus;
using namespace log4cplus::helpers;


//////////////////////////////////////////////////////////////////////////////
// log4cplus::helpers::AppenderAttachableImpl ctor and dtor
//////////////////////////////////////////////////////////////////////////////

AppenderAttachableImpl::AppenderAttachableImpl()
 : mutex(LOG4CPLUS_MUTEX_CREATE)
{
}


AppenderAttachableImpl::~AppenderAttachableImpl()
{
   LOG4CPLUS_MUTEX_FREE( mutex );
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::helpers::AppenderAttachableImpl public methods
///////////////////////////////////////////////////////////////////////////////

void
AppenderAttachableImpl::addAppender(SharedAppenderPtr newAppender)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        if(newAppender == NULL) {
            getLogLog().warn( LOG4CPLUS_TEXT("Tried to add NULL appender") );
            return;
        }

        ListType::iterator it = 
            std::find(appenderList.begin(), appenderList.end(), newAppender);
        if(it == appenderList.end()) {
            appenderList.push_back(newAppender);
        }
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}



AppenderAttachableImpl::ListType
AppenderAttachableImpl::getAllAppenders()
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        return appenderList;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}



SharedAppenderPtr 
AppenderAttachableImpl::getAppender(const log4cplus::tstring& name)
{
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
}



void 
AppenderAttachableImpl::removeAllAppenders()
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        appenderList.erase(appenderList.begin(), appenderList.end());
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}



void 
AppenderAttachableImpl::removeAppender(SharedAppenderPtr appender)
{
    if(appender == NULL) {
        getLogLog().warn( LOG4CPLUS_TEXT("Tried to remove NULL appender") );
        return;
    }

    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        ListType::iterator it =
            std::find(appenderList.begin(), appenderList.end(), appender);
        if(it != appenderList.end()) {
            appenderList.erase(it);
        }
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}



void 
AppenderAttachableImpl::removeAppender(const log4cplus::tstring& name)
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


