// Module:  Log4CPLUS
// File:    threads.cxx
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

#ifndef LOG4CPLUS_SINGLE_THREADED

#include <log4cplus/helpers/threads.h>
#include <log4cplus/streams.h>
#include <log4cplus/ndc.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/helpers/timehelper.h>

#include <exception>
#include <stdexcept>
#include <errno.h>

#if defined(LOG4CPLUS_USE_PTHREADS)
#  include <sched.h>
#  include <signal.h>
#elif defined (LOG4CPLUS_USE_WIN32_THREADS)
#  include <process.h> 
#endif

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;


///////////////////////////////////////////////////////////////////////////////
// public methods
///////////////////////////////////////////////////////////////////////////////

LOG4CPLUS_MUTEX_PTR_DECLARE 
log4cplus::thread::createNewMutex()
{
#if defined(LOG4CPLUS_USE_PTHREADS)
    pthread_mutex_t* m = new pthread_mutex_t();
    pthread_mutex_init(m, NULL);
#elif defined(LOG4CPLUS_USE_WIN32_THREADS)
    CRITICAL_SECTION* m = new CRITICAL_SECTION();
    InitializeCriticalSection(m);
#endif
    return m;
}


void 
log4cplus::thread::deleteMutex(LOG4CPLUS_MUTEX_PTR_DECLARE m)
{
#if defined(LOG4CPLUS_USE_PTHREADS)
    pthread_mutex_destroy(m);
#elif defined(LOG4CPLUS_USE_WIN32_THREADS)
    DeleteCriticalSection(m);
#endif
    delete m;
}



#if defined(LOG4CPLUS_USE_PTHREADS)
pthread_key_t*
log4cplus::thread::createPthreadKey()
{
    pthread_key_t* key = new pthread_key_t();
    pthread_key_create(key, NULL);
    return key;
}
#endif


#ifndef LOG4CPLUS_SINGLE_THREADED
void
log4cplus::thread::blockAllSignals()
{
#if defined (LOG4CPLUS_USE_PTHREADS)
    // Block all signals.
    sigset_t signal_set;
    sigfillset (&signal_set);
    pthread_sigmask (SIG_BLOCK, &signal_set, 0);
#endif    
}
#endif // LOG4CPLUS_SINGLE_THREADED


void
log4cplus::thread::yield()
{
#if defined(LOG4CPLUS_USE_PTHREADS)
    ::sched_yield();
#elif defined(LOG4CPLUS_USE_WIN32_THREADS)
    ::Sleep(0);
#endif
}


log4cplus::tstring 
log4cplus::thread::getCurrentThreadName()
{
#if 1
    log4cplus::tostringstream tmp;
    tmp << LOG4CPLUS_GET_CURRENT_THREAD;

    return tmp.str();
#else
    return convertIntegerToString(LOG4CPLUS_GET_CURRENT_THREAD);
#endif
}



#if defined(LOG4CPLUS_USE_PTHREADS)
    void* 
    log4cplus::thread::threadStartFunc(void* arg)
#elif defined(LOG4CPLUS_USE_WIN32_THREADS)
    unsigned WINAPI
    log4cplus::thread::threadStartFunc(void * arg)
#endif
{
    blockAllSignals ();
    SharedObjectPtr<LogLog> loglog = LogLog::getLogLog();
    if(arg == NULL) {
        loglog->error(LOG4CPLUS_TEXT("log4cplus::thread::threadStartFunc()- arg is NULL"));
    }
    else {
        AbstractThread* ptr = static_cast<AbstractThread*>(arg);
        log4cplus::helpers::SharedObjectPtr<AbstractThread> thread(ptr);
        try {
            thread->run();
        }
        catch(std::exception& e) {
            tstring err = LOG4CPLUS_TEXT("log4cplus::thread::threadStartFunc()- run() terminated with an exception: ");
            err += LOG4CPLUS_C_STR_TO_TSTRING(e.what());
            loglog->warn(err);
        }
        catch(...) {
            loglog->warn(LOG4CPLUS_TEXT("log4cplus::thread::threadStartFunc()- run() terminated with an exception."));
        }
        thread->running = false;
        getNDC().remove();
    }

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::thread::AbstractThread ctor and dtor
///////////////////////////////////////////////////////////////////////////////

log4cplus::thread::AbstractThread::AbstractThread()
: running(false)
{
}



log4cplus::thread::AbstractThread::~AbstractThread()
{
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::thread::AbstractThread public methods
///////////////////////////////////////////////////////////////////////////////

void
log4cplus::thread::AbstractThread::start()
{
    running = true;
#if defined(LOG4CPLUS_USE_PTHREADS)
    if( pthread_create(&threadId, NULL, threadStartFunc, this) )
        throw std::runtime_error("Thread creation was not successful");
#elif defined(LOG4CPLUS_USE_WIN32_THREADS)
    HANDLE h = reinterpret_cast<HANDLE>(
        _beginthreadex (0, 0, threadStartFunc, this, 0, 0));
    if (! h)
        throw std::runtime_error("Thread creation was not successful");
    CloseHandle(h);
#endif
}

#endif // LOG4CPLUS_SINGLE_THREADED

