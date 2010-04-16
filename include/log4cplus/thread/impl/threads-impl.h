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

#ifndef LOG4CPLUS_IMPL_THREADS_IMPL_HEADER_
#define LOG4CPLUS_IMPL_THREADS_IMPL_HEADER_

#include <log4cplus/config.hxx>
#include <log4cplus/tstring.h>
#include <log4cplus/helpers/pointer.h>
#include <log4cplus/thread/threads.h>

#if ! defined (INSIDE_LOG4CPLUS)
#  error "This header must not be be used outside log4cplus' implementation files."
#endif


namespace log4cplus { namespace thread { namespace impl {


#ifndef LOG4CPLUS_SINGLE_THREADED


struct ThreadStart
{
#  ifdef LOG4CPLUS_USE_PTHREADS
static void* threadStartFuncWorker(void *);
#  elif defined(LOG4CPLUS_USE_WIN32_THREADS)
static unsigned threadStartFuncWorker(void *);
#  endif
};


/**
 * There are many cross-platform C++ Threading libraries.  The goal of
 * this class is not to replace (or match in functionality) those
 * libraries.  The goal of this class is to provide a simple Threading
 * class with basic functionality.
 */
class Thread
    : public ThreadImplBase
{
public:
    Thread();
    bool isRunning() const;
    LOG4CPLUS_THREAD_KEY_TYPE getThreadId() const;
    LOG4CPLUS_THREAD_HANDLE_TYPE getThreadHandle () const;
    void start();
    void join () const;

protected:
    // Force objects to be constructed on the heap
    virtual ~Thread();
    virtual void run() = 0;

private:
    bool running;

    // Friends.
    friend struct ThreadStart;

#  ifdef LOG4CPLUS_USE_PTHREADS
    pthread_t handle;

#  elif defined(LOG4CPLUS_USE_WIN32_THREADS)
    HANDLE handle;
    unsigned thread_id;

#  endif

    // Disallow copying of instances of this class.
    Thread(const Thread&);
    Thread& operator=(const Thread&);
};

typedef helpers::SharedObjectPtr<Thread> ThreadPtr;


#endif // LOG4CPLUS_SINGLE_THREADED


} } } // namespace log4cplus { namespace thread { namespace impl {


#endif // LOG4CPLUS_IMPL_THREADS_IMPL_HEADER_
