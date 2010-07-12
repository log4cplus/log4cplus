// Module:  Log4CPLUS
// File:    threads.h
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright 2001-2010 Tad E. Smith
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

/** @file */

#ifndef _LOG4CPLUS_THREADS_HEADER_
#define _LOG4CPLUS_THREADS_HEADER_

#include <log4cplus/config.hxx>
#include <log4cplus/tstring.h>
#include <log4cplus/helpers/pointer.h>


namespace log4cplus { namespace thread {

/**
 * This is used to lock a mutex.  The dtor unlocks the mutex.
 */
class Guard
{
public:
    /** "locks" <code>mutex</code>. */
    Guard(LOG4CPLUS_MUTEX_PTR_DECLARE mutex)
        : _mutex (mutex)
    {
        LOG4CPLUS_MUTEX_LOCK( _mutex );
    }

    /** "unlocks" <code>mutex</code>. */
    ~Guard()
    {
        LOG4CPLUS_MUTEX_UNLOCK( _mutex );
    }

private:
    LOG4CPLUS_MUTEX_PTR_DECLARE _mutex;

    // disable copy
    Guard(const Guard&);
    Guard& operator=(const Guard&);
};


#ifndef LOG4CPLUS_SINGLE_THREADED

LOG4CPLUS_EXPORT void blockAllSignals();
LOG4CPLUS_EXPORT void yield();
LOG4CPLUS_EXPORT tstring getCurrentThreadName();


struct ThreadStart;


/**
 * There are many cross-platform C++ Threading libraries.  The goal of
 * this class is not to replace (or match in functionality) those
 * libraries.  The goal of this class is to provide a simple Threading
 * class with basic functionality.
 */
class LOG4CPLUS_EXPORT AbstractThread
    : public virtual log4cplus::helpers::SharedObject
{
public:
    AbstractThread();
    bool isRunning() const { return (flags & fRUNNING) != 0; }
    LOG4CPLUS_THREAD_KEY_TYPE getThreadId() const;
    LOG4CPLUS_THREAD_HANDLE_TYPE getThreadHandle () const;
    virtual void start();
    void join ();

protected:
    // Force objects to be constructed on the heap
    virtual ~AbstractThread();
    virtual void run() = 0;

private:
    enum Flags
    {
        fRUNNING  = 0x01,
        fJOINED   = 0x02
    };

    unsigned flags;

    // Friends.
    friend struct ThreadStart;

#  ifdef LOG4CPLUS_USE_PTHREADS
    pthread_t handle;

#  elif defined(LOG4CPLUS_USE_WIN32_THREADS)
    HANDLE handle;
#    if defined (_WIN32_WCE)
    DWORD thread_id;
#    else
    unsigned thread_id;
#    endif

#  endif

    // Disallow copying of instances of this class.
    AbstractThread(const AbstractThread&);
    AbstractThread& operator=(const AbstractThread&);
};

typedef helpers::SharedObjectPtr<AbstractThread> AbstractThreadPtr;


#endif // LOG4CPLUS_SINGLE_THREADED


} } // namespace log4cplus { namespace thread {


#endif // _LOG4CPLUS_THREADS_HEADER_

