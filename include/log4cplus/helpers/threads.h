// Module:  Log4CPLUS
// File:    threads.h
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

/** @file */

#ifndef _LOG4CPLUS_THREADS_HEADER_
#define _LOG4CPLUS_THREADS_HEADER_

#include <log4cplus/config.hxx>
#include <log4cplus/tstring.h>
#include <log4cplus/helpers/sleep.h>
#include <log4cplus/helpers/pointer.h>
#include <memory>


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

#  ifdef LOG4CPLUS_USE_PTHREADS
void* threadStartFunc(void*);
#  elif defined(LOG4CPLUS_USE_WIN32_THREADS)
unsigned WINAPI threadStartFunc(void *);
#  endif


LOG4CPLUS_EXPORT void yield();
LOG4CPLUS_EXPORT log4cplus::tstring const & getCurrentThreadName();

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
    bool isRunning() const { return running; }
    LOG4CPLUS_THREAD_KEY_TYPE getThreadId() const;
    LOG4CPLUS_THREAD_HANDLE_TYPE getThreadHandle () const;
    virtual void start();
    void join () const;

protected:
    // Force objects to be constructed on the heap
    virtual ~AbstractThread();
    virtual void run() = 0;

private:
    bool running;

    // Friends.
#  ifdef LOG4CPLUS_USE_PTHREADS
    friend void* threadStartFunc(void*);
    pthread_t handle;

#  elif defined(LOG4CPLUS_USE_WIN32_THREADS)
    HANDLE handle;
    friend unsigned WINAPI threadStartFunc(void *);

#  endif

    // Disallow copying of instances of this class.
    AbstractThread(const AbstractThread&);
    AbstractThread& operator=(const AbstractThread&);
};

typedef helpers::SharedObjectPtr<AbstractThread> AbstractThreadPtr;


#endif // LOG4CPLUS_SINGLE_THREADED


} } // namespace log4cplus { namespace thread {


#endif // _LOG4CPLUS_THREADS_HEADER_

