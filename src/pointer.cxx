// Module:  Log4CPLUS
// File:    pointer.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright 2001-2015 Tad E. Smith
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
#include <log4cplus/thread/threads.h>
#include <log4cplus/thread/impl/syncprims-impl.h>
#include <log4cplus/config/windowsh-inc.h>
#include <cassert>
#if defined (LOG4CPLUS_HAVE_INTRIN_H)
#include <intrin.h>
#endif
#if defined (LOG4CPLUS_HAVE_ATOMIC_H)
#include <atomic.h>
#endif


namespace log4cplus { namespace helpers {


///////////////////////////////////////////////////////////////////////////////
// log4cplus::helpers::SharedObject dtor
///////////////////////////////////////////////////////////////////////////////

SharedObject::~SharedObject()
{
    assert(count == 0);
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::helpers::SharedObject public methods
///////////////////////////////////////////////////////////////////////////////

void
SharedObject::addReference() const
{
#if defined (LOG4CPLUS_SINGLE_THREADED)
    ++count;

#elif defined (LOG4CPLUS_HAVE_CXX11_ATOMICS)
    std::atomic_fetch_add_explicit (&count, 1u,
        std::memory_order_relaxed);

#elif defined (LOG4CPLUS_HAVE___ATOMIC_ADD_FETCH)
    __atomic_add_fetch (&count, 1u, __ATOMIC_RELAXED);

#elif defined (LOG4CPLUS_HAVE___SYNC_ADD_AND_FETCH)
    __sync_add_and_fetch (&count, 1);

#elif defined (LOG4CPLUS_HAVE_ATOMIC_INC_UINT)
    atomic_inc_uint (&count);

#elif defined (_WIN32) && defined (LOG4CPLUS_HAVE_INTRIN_H)
    _InterlockedIncrement (&count);

#elif defined (_WIN32)
    InterlockedIncrement (&count);

#else
    thread::MutexGuard guard (access_mutex);
    ++count;

#endif
}


void
SharedObject::removeReference() const
{
    assert (count > 0);
    bool destroy;

#if defined (LOG4CPLUS_SINGLE_THREADED)
    destroy = --count == 0;

#elif defined (LOG4CPLUS_HAVE_CXX11_ATOMICS)
    destroy = std::atomic_fetch_sub_explicit (&count, 1u,
        std::memory_order_release) == 1;
    if (LOG4CPLUS_UNLIKELY (destroy))
        std::atomic_thread_fence (std::memory_order_acquire);

#elif defined (LOG4CPLUS_HAVE___ATOMIC_SUB_FETCH)
    destroy = __atomic_sub_fetch (&count, 1u,
        __ATOMIC_RELEASE) == 0;
    if (LOG4CPLUS_UNLIKELY (destroy))
        __atomic_thread_fence (__ATOMIC_ACQUIRE);

#elif defined (LOG4CPLUS_HAVE___SYNC_SUB_AND_FETCH)
    destroy = __sync_sub_and_fetch (&count, 1) == 0;

#elif defined (LOG4CPLUS_HAVE_ATOMIC_DEC_UINT_NV)
    destroy = atomic_dec_uint_nv (&count) == 0;
    if (destroy)
        membar_exit ();

#elif defined (_WIN32) && defined (LOG4CPLUS_HAVE_INTRIN_H)
    destroy = _InterlockedDecrement (&count) == 0;

#elif defined (_WIN32)
    destroy = InterlockedDecrement (&count) == 0;

#else
    {
        thread::MutexGuard guard (access_mutex);
        destroy = --count == 0;
    }

#endif
    if (LOG4CPLUS_UNLIKELY (destroy))
        delete this;
}


} } // namespace log4cplus { namespace helpers
