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
// $Log: not supported by cvs2svn $
// Revision 1.6  2003/06/13 17:49:30  tcsmith
// Changed to use the old style C headers.
//
// Revision 1.5  2003/04/19 23:04:32  tcsmith
// Fixed UNICODE support.
//
// Revision 1.4  2003/04/18 21:13:25  tcsmith
// Converted from std::string to log4cplus::tstring.
//
// Revision 1.3  2003/04/03 01:20:23  tcsmith
// Standardized the formatting.
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
    if(destroyed) {
        assert(!destroyed);
    }
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex )
        if(destroyed) {
            assert(!destroyed);
        }
        if(--count == 0) destroyed = true;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
    if (destroyed) delete this;
}


