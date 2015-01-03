// -*- C++ -*-
//  Copyright (C) 2013-2015, Vaclav Zeman. All rights reserved.
//  
//  Redistribution and use in source and binary forms, with or without modifica-
//  tion, are permitted provided that the following conditions are met:
//  
//  1. Redistributions of  source code must  retain the above copyright  notice,
//     this list of conditions and the following disclaimer.
//  
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  
//  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED WARRANTIES,
//  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS  FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN NO  EVENT SHALL  THE
//  APACHE SOFTWARE  FOUNDATION  OR ITS CONTRIBUTORS  BE LIABLE FOR  ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLU-
//  DING, BUT NOT LIMITED TO, PROCUREMENT  OF SUBSTITUTE GOODS OR SERVICES; LOSS
//  OF USE, DATA, OR  PROFITS; OR BUSINESS  INTERRUPTION)  HOWEVER CAUSED AND ON
//  ANY  THEORY OF LIABILITY,  WHETHER  IN CONTRACT,  STRICT LIABILITY,  OR TORT
//  (INCLUDING  NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT OF THE  USE OF
//  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//! @file
//! This file contains implementations of synchronization
//! primitives using the Win32 API. It does not contain any include
//! guards because it is only a fragment to be included by
//! syncprims.h.

#include <stdexcept>
#include <chrono>
#include <algorithm>


namespace log4cplus { namespace thread { namespace impl {


//
//
//


inline
Mutex::Mutex (log4cplus::thread::Mutex::Type)
{ }


inline
Mutex::~Mutex ()
{ }


inline
void
Mutex::lock () const
{
    mtx.lock ();
}


inline
void
Mutex::unlock () const
{
    mtx.unlock ();
}


//
//
//

inline
Semaphore::Semaphore (unsigned max_, unsigned initial)
    : max (max_)
    , val ((std::min) (max, initial))
{ }


inline
Semaphore::~Semaphore ()
{ }


inline
void
Semaphore::unlock () const
{
    std::lock_guard<std::mutex> guard (mtx);

    if (val >= max)
        LOG4CPLUS_THROW_RTE ("Semaphore::unlock(): val >= max");

    ++val;
    cv.notify_all ();
}


inline
void
Semaphore::lock () const
{
    std::unique_lock<std::mutex> guard (mtx);

    while (val == 0)        
        cv.wait (guard);

    --val;

    if (LOG4CPLUS_UNLIKELY(val >= max))
        LOG4CPLUS_THROW_RTE ("Semaphore::unlock(): val >= max");
}


//
//
//


inline
FairMutex::FairMutex ()
    : sem (1, 1)
{ }


inline
FairMutex::~FairMutex ()
{ }


inline
void
FairMutex::lock () const
{
    sem.lock ();
}


inline
void
FairMutex::unlock () const
{
    sem.unlock ();
}


//
//
//

inline
ManualResetEvent::ManualResetEvent (bool sig)
    : sigcount (0)
    , signaled (sig)
{ }


inline
ManualResetEvent::~ManualResetEvent ()
{ }


inline
void
ManualResetEvent::signal () const
{
    std::unique_lock<std::mutex> guard (mtx);

    signaled = true;
    sigcount += 1;
    cv.notify_all ();
}


inline
void
ManualResetEvent::wait () const
{
    std::unique_lock<std::mutex> guard (mtx);

    if (! signaled)
    {
        unsigned prev_count = sigcount;
        do
        {
            cv.wait (guard);
        }
        while (prev_count == sigcount);
    }
}


inline
bool
ManualResetEvent::timed_wait (unsigned long msec) const
{
    std::unique_lock<std::mutex> guard (mtx);

    if (! signaled)
    {
        unsigned prev_count = sigcount;

        std::chrono::steady_clock::time_point const wait_until_time
            = std::chrono::steady_clock::now ()
            + std::chrono::milliseconds (msec);

        do
        {
            int ret = static_cast<int>(
                cv.wait_until (guard, wait_until_time));
            switch (ret)
            {
            case static_cast<int>(std::cv_status::no_timeout):
                break;

            case static_cast<int>(std::cv_status::timeout):
                return false;

            default:
                guard.unlock ();
                guard.release ();
                LOG4CPLUS_THROW_RTE ("ManualResetEvent::timed_wait");
            }
        }
        while (prev_count == sigcount);
    }

    return true;
}


inline
void
ManualResetEvent::reset () const
{
    std::lock_guard<std::mutex> guard (mtx);

    signaled = false;
}


//
//
//

#include "log4cplus/thread/impl/syncprims-pmsm.h"


} } } // namespace log4cplus { namespace thread { namespace impl {
