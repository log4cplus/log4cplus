// Module:  Log4CPLUS
// File:    threadkey.cxx
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

#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/threadkey.h>
#include <exception>
#include <stdexcept>

using namespace log4cplus::helpers;


void*
log4cplus::thread::NonCleaningThreadKey::get()
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        MapType::iterator it = map.find(LOG4CPLUS_GET_CURRENT_THREAD);
        if(it != map.end()) {
            return it->second;
        }
        else {
            return NULL;
        }
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}


void
log4cplus::thread::NonCleaningThreadKey::set(void *arg)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        LOG4CPLUS_THREAD_KEY_TYPE threadKey = LOG4CPLUS_GET_CURRENT_THREAD;
        MapType::iterator it = map.find(threadKey);
        if(it != map.end()) {
            it->second = arg;
        }
        else {
            bool inserted = map.insert(std::make_pair(threadKey, arg)).second;
            if(!inserted) {
                getLogLog().error("NonCleaningThreadKey::set()- Insert failed");
                throw std::runtime_error("NonCleaningThreadKey::set()- Insert failed");
            }
        }
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}



void
log4cplus::thread::NonCleaningThreadKey::cleanEntry()
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        map.erase(LOG4CPLUS_GET_CURRENT_THREAD);
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}


