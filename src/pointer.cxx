// Module:  Log4CPLUS
// File:    pointer.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright 2001-2009 Tad E. Smith
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

#include <log4cplus/streams.h>
#include <log4cplus/helpers/pointer.h>
#include <log4cplus/helpers/threads.h>
#include <assert.h>


namespace log4cplus { namespace helpers {


///////////////////////////////////////////////////////////////////////////////
// log4cplus::helpers::SharedObject dtor
///////////////////////////////////////////////////////////////////////////////

SharedObject::~SharedObject()
{
    assert(count == 0);
    LOG4CPLUS_MUTEX_FREE( access_mutex );
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::helpers::SharedObject public methods
///////////////////////////////////////////////////////////////////////////////

void
SharedObject::addReference() const
{
#if ! defined(LOG4CPLUS_SINGLE_THREADED) \
    && defined (LOG4CPLUS_HAVE___SYNC_ADD_AND_FETCH)
    __sync_add_and_fetch (&count, 1);

#elif ! defined(LOG4CPLUS_SINGLE_THREADED) \
    && defined (WIN32)
    InterlockedIncrement (&count);

#else
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex )
        ++count;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;

#endif
}


void
SharedObject::removeReference() const
{
    bool destroy = false;
#if ! defined(LOG4CPLUS_SINGLE_THREADED) \
    && defined (LOG4CPLUS_HAVE___SYNC_SUB_AND_FETCH)
    destroy = __sync_sub_and_fetch (&count, 1) == 0;

#elif ! defined(LOG4CPLUS_SINGLE_THREADED) && defined (WIN32)
    destroy = InterlockedDecrement (&count) == 0;

#else
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex );
        assert (count > 0);
        if (--count == 0)
            destroy = true;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;

#endif
    if (destroy)
        delete this;
}


} } // namespace log4cplus { namespace helpers
