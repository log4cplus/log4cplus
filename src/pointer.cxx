// Module:  Log4CPLUS
// File:    pointer.cxx
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
#include <log4cplus/helpers/pointer.h>
#include <cassert>
#include <cstdio>

using namespace log4cplus::helpers;


void
log4cplus::helpers::throwNullPointerException(const char* file, int line)
{
    char *buffer = new char[200];
    if(file) {
        sprintf(buffer, "NullPointer: file=%s line=%u", file, line);
    }
    else {
        sprintf(buffer, "NullPointer: file=UNKNOWN line=%u", line);
    }

    std::string s(buffer);
    delete [] buffer;
    throw NullPointerException(s);
}



SharedObject::~SharedObject()
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex )
        if(!destroyed) {
            getLogLog().error("SharedObject::~SharedObject()- This object MUST be destroyed");
            assert(destroyed);
        }
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
    LOG4CPLUS_MUTEX_FREE( access_mutex );
}


void
SharedObject::addReference()
{
    if(destroyed) {
        getLogLog().error("SharedObject::addReference()- This object is already destroyed");
        assert(!destroyed);
    }
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex )
        if(destroyed) {
            getLogLog().error("SharedObject::addReference()- This object is already destroyed");
            assert(!destroyed);
        }
        ++count;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}


void
SharedObject::removeReference()
{
    if(destroyed) {
        getLogLog().error("SharedObject::removeReference()- This object is already destroyed");
        assert(!destroyed);
    }
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex )
        if(destroyed) {
            getLogLog().error("SharedObject::removeReference()- This object is already destroyed");
            assert(!destroyed);
        }
        if(--count == 0) destroyed = true;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
    if (destroyed) delete this;
}


