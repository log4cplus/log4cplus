// Module:  Log4CPLUS
// File:    pointer.cxx
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

#include <log4cplus/streams.h>
#include <log4cplus/helpers/pointer.h>
#include <log4cplus/helpers/threads.h>
#include <assert.h>
#include <stdio.h>

using namespace std;
using namespace log4cplus::helpers;


///////////////////////////////////////////////////////////////////////////////
// public methods
///////////////////////////////////////////////////////////////////////////////

void
log4cplus::helpers::throwNullPointerException(const char* file, int line)
{
    tostringstream buf;
    buf << LOG4CPLUS_TEXT("NullPointer: file=")
        << file
        << LOG4CPLUS_TEXT(" line=")
        << line
        << endl;

    throw NullPointerException(LOG4CPLUS_TSTRING_TO_STRING(buf.str()));
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::helpers::SharedObject dtor
///////////////////////////////////////////////////////////////////////////////

SharedObject::~SharedObject()
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex )
        if(!destroyed) {
            assert(destroyed);
        }
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
    LOG4CPLUS_MUTEX_FREE( access_mutex );
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::helpers::SharedObject public methods
///////////////////////////////////////////////////////////////////////////////

void
SharedObject::addReference()
{
    if(destroyed) {
        assert(!destroyed);
    }
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex )
        if(destroyed) {
            assert(!destroyed);
        }
        ++count;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}


void
SharedObject::removeReference()
{
    bool destroy = false;
    assert(!destroyed);
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex );
    assert(!destroyed);
    if(--count == 0)
        destroy = destroyed = true;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
    if (destroy)
        delete this;
}
