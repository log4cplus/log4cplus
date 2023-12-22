// -*- C++ -*-
//  Copyright (C) 2010-2017, Vaclav Haisman. All rights reserved.
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

#ifndef LOG4CPLUS_THREAD_SYNCPRIMS_H
#define LOG4CPLUS_THREAD_SYNCPRIMS_H

#include <log4cplus/config.hxx>

#if defined (LOG4CPLUS_HAVE_PRAGMA_ONCE)
#pragma once
#endif

#include <mutex>
#include <condition_variable>
#include <log4cplus/internal/threadsafetyanalysis.h>


namespace log4cplus { namespace thread {


template <typename SyncPrim>
class LOG4CPLUS_TSA_SCOPED_CAPABILITY SyncGuard
{
public:
    SyncGuard ();
    SyncGuard (SyncPrim const & sp) LOG4CPLUS_TSA_ACQUIRE (sp);
    ~SyncGuard () LOG4CPLUS_TSA_RELEASE ();
    SyncGuard (SyncGuard const &) = delete;
    SyncGuard & operator = (SyncGuard const &) = delete;


    void lock () LOG4CPLUS_TSA_ACQUIRE ();
    void unlock () LOG4CPLUS_TSA_RELEASE ();
    void attach (SyncPrim const & sp) LOG4CPLUS_TSA_REQUIRES (sp);
    void attach_and_lock (SyncPrim const & sp) LOG4CPLUS_TSA_ACQUIRE (sp);
    void detach ();

private:
    SyncPrim const * sp;
};


class LOG4CPLUS_EXPORT LOG4CPLUS_TSA_CAPABILITY ("mutex") Mutex
{
public:
    Mutex ();
    ~Mutex ();
    Mutex (Mutex const &) = delete;
    Mutex & operator = (Mutex const &) = delete;

    void lock () const LOG4CPLUS_TSA_ACQUIRE ();
    void unlock () const LOG4CPLUS_TSA_RELEASE ();

private:
    LOG4CPLUS_THREADED (mutable std::recursive_mutex mtx;)
};


typedef SyncGuard<Mutex> MutexGuard;


class LOG4CPLUS_EXPORT Semaphore
{
public:
    Semaphore (unsigned max, unsigned initial);
    ~Semaphore ();
    Semaphore (Semaphore const &) = delete;
    Semaphore & operator = (Semaphore const &) = delete;

    void lock () const;
    void unlock () const;

private:
#if ! defined (LOG4CPLUS_SINGLE_THREADED)
    mutable std::mutex mtx;
    mutable std::condition_variable cv;
    mutable unsigned max_;
    mutable unsigned val;
#endif
};


typedef SyncGuard<Semaphore> SemaphoreGuard;


class LOG4CPLUS_EXPORT ManualResetEvent
{
public:
    explicit ManualResetEvent (bool = false);
    ~ManualResetEvent ();
    ManualResetEvent (ManualResetEvent const &) = delete;
    ManualResetEvent & operator = (ManualResetEvent const &) = delete;

    void signal () const;
    void wait () const;
    bool timed_wait (unsigned long msec) const;
    void reset () const;

private:
#if ! defined (LOG4CPLUS_SINGLE_THREADED)
    mutable std::mutex mtx;
    mutable std::condition_variable cv;
    mutable bool signaled;
    mutable unsigned sigcount;
#endif
};


class SharedMutexImplBase
{
protected:
    ~SharedMutexImplBase ();
};


template <typename SyncPrim, void (SyncPrim:: * lock_func) () const,
    void (SyncPrim:: * unlock_func) () const>
class SyncGuardFunc
{
public:
    SyncGuardFunc (SyncPrim const &);
    ~SyncGuardFunc ();
    SyncGuardFunc (SyncGuardFunc const &) = delete;
    SyncGuardFunc & operator = (SyncGuardFunc const &) = delete;

    void lock ();
    void unlock ();
    void attach (SyncPrim const &);
    void detach ();

private:
    SyncPrim const * sp;
};


class LOG4CPLUS_EXPORT LOG4CPLUS_TSA_CAPABILITY ("mutex") SharedMutex
{
public:
    SharedMutex ();
    ~SharedMutex ();
    SharedMutex (SharedMutex const &) = delete;
    SharedMutex & operator = (SharedMutex const &) = delete;

    void rdlock () const LOG4CPLUS_TSA_ACQUIRE_SHARED ();
    void rdunlock () const LOG4CPLUS_TSA_RELEASE_SHARED ();

    void wrlock () const LOG4CPLUS_TSA_ACQUIRE ();
    void wrunlock () const LOG4CPLUS_TSA_RELEASE ();

private:
    SharedMutexImplBase * sm;
};


typedef SyncGuardFunc<SharedMutex, &SharedMutex::rdlock,
    &SharedMutex::rdunlock> SharedMutexReaderGuard;


typedef SyncGuardFunc<SharedMutex, &SharedMutex::wrlock,
    &SharedMutex::wrunlock> SharedMutexWriterGuard;


//
//
//

template <typename SyncPrim>
inline
SyncGuard<SyncPrim>::SyncGuard ()
    : sp (nullptr)
{ }


template <typename SyncPrim>
inline
SyncGuard<SyncPrim>::SyncGuard (SyncPrim const & m)
    : sp (&m)
{
    sp->lock ();
}


template <typename SyncPrim>
inline
SyncGuard<SyncPrim>::~SyncGuard ()
{
    if (sp)
        sp->unlock ();
}


template <typename SyncPrim>
inline
void
SyncGuard<SyncPrim>::lock ()
{
    sp->lock ();
}


template <typename SyncPrim>
inline
void
SyncGuard<SyncPrim>::unlock ()
{
    sp->unlock ();
}


template <typename SyncPrim>
inline
void
SyncGuard<SyncPrim>::attach (SyncPrim const & m)
{
    sp = &m;
}


template <typename SyncPrim>
inline
void
SyncGuard<SyncPrim>::attach_and_lock (SyncPrim const & m)
{
    attach (m);
    try
    {
        lock();
    }
    catch (...)
    {
        detach ();
        throw;
    }
}


template <typename SyncPrim>
inline
void
SyncGuard<SyncPrim>::detach ()
{
    sp = 0;
}


//
//
//

template <typename SyncPrim, void (SyncPrim:: * lock_func) () const,
    void (SyncPrim:: * unlock_func) () const>
inline
SyncGuardFunc<SyncPrim, lock_func, unlock_func>::SyncGuardFunc (SyncPrim const & m)
    : sp (&m)
{
    (sp->*lock_func) ();
}


template <typename SyncPrim, void (SyncPrim:: * lock_func) () const,
    void (SyncPrim:: * unlock_func) () const>
inline
SyncGuardFunc<SyncPrim, lock_func, unlock_func>::~SyncGuardFunc ()
{
    if (sp)
        (sp->*unlock_func) ();
}


template <typename SyncPrim, void (SyncPrim:: * lock_func) () const,
    void (SyncPrim:: * unlock_func) () const>
inline
void
SyncGuardFunc<SyncPrim, lock_func, unlock_func>::lock ()
{
    (sp->*lock_func) ();
}


template <typename SyncPrim, void (SyncPrim:: * lock_func) () const,
    void (SyncPrim:: * unlock_func) () const>
inline
void
SyncGuardFunc<SyncPrim, lock_func, unlock_func>::unlock ()
{
    (sp->*unlock_func) ();
}


template <typename SyncPrim, void (SyncPrim:: * lock_func) () const,
    void (SyncPrim:: * unlock_func) () const>
inline
void
SyncGuardFunc<SyncPrim, lock_func, unlock_func>::attach (SyncPrim const & m)
{
    sp = &m;
}


template <typename SyncPrim, void (SyncPrim:: * lock_func) () const,
    void (SyncPrim:: * unlock_func) () const>
inline
void
SyncGuardFunc<SyncPrim, lock_func, unlock_func>::detach ()
{
    sp = 0;
}


} } // namespace log4cplus { namespace thread {


#endif // LOG4CPLUS_THREAD_SYNCPRIMS_H
