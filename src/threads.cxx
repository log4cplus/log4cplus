// Module:  Log4CPLUS
// File:    threads.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright (C) The Apache Software Foundation. All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/threads.h>
#include <exception>
#include <sstream>
#include <stdexcept>
#include <errno.h>
#include <time.h>

#ifdef LOG4CPLUS_USE_PTHREADS
#    include <sched.h>
#endif

using namespace log4cplus::helpers;


#ifndef LOG4CPLUS_SINGLE_THREADED


LOG4CPLUS_MUTEX_PTR_DECLARE 
log4cplus::thread::createNewMutex()
{
#ifdef LOG4CPLUS_USE_PTHREADS
    pthread_mutex_t* m = new pthread_mutex_t();
    pthread_mutex_init(m, NULL);
#endif
#ifdef LOG4CPLUS_USE_WIN32_THREADS
    CRITICAL_SECTION* m = new CRITICAL_SECTION();
    InitializeCriticalSection(m);
#endif
    return m;
}


void 
log4cplus::thread::deleteMutex(LOG4CPLUS_MUTEX_PTR_DECLARE m)
{
#ifdef LOG4CPLUS_USE_PTHREADS
    pthread_mutex_destroy(m);
#endif
#ifdef LOG4CPLUS_USE_WIN32_THREADS
    DeleteCriticalSection(m);
#endif
    delete m;
}



#ifdef LOG4CPLUS_USE_PTHREADS
pthread_key_t*
log4cplus::thread::createPthreadKey()
{
    pthread_key_t* key = new pthread_key_t();
    pthread_key_create(key, NULL);
    return key;
}
#endif


log4cplus::thread::Guard::~Guard()
{
    try {
        LOG4CPLUS_MUTEX_UNLOCK( _mutex );
    }
    catch(exception& e) {
        helpers::getLogLog().error(std::string("**** Guard::~Guard() exception: ") + e.what());
        throw;
    }
    catch(...) {
        helpers::getLogLog().error("**** Guard::~Guard() exception!!!!");
        throw;
    }
}



void
log4cplus::thread::yield()
{
#ifdef LOG4CPLUS_USE_PTHREADS
    sched_yield();
#endif
#ifdef LOG4CPLUS_USE_WIN32_THREADS
    Sleep(0);
#endif
}


#define MILLIS_TO_NANOS 1000000
#define SEC_TO_MILLIS 1000
#define MAX_SLEEP_SECONDS (DWORD)4294966        // (2**32-2)/1000

void
log4cplus::thread::sleep(unsigned long secs, unsigned long nanosecs)
{
#ifdef LOG4CPLUS_USE_PTHREADS
    timespec sleep_time = { secs, nanosecs };
    timespec remain;
    while (nanosleep(&sleep_time, &remain)) {
        if (errno == EINTR) {
            sleep_time.tv_sec  = remain.tv_sec;
            sleep_time.tv_nsec = remain.tv_nsec;               
            continue;
        }
        else {
            return;
        }
    }
#endif
#ifdef LOG4CPLUS_USE_WIN32_THREADS
    DWORD nano_millis = nanosecs / static_cast<unsigned long>(MILLIS_TO_NANOS);
    if (secs <= MAX_SLEEP_SECONDS) {
        Sleep((secs * SEC_TO_MILLIS) + nano_millis);
        return;
    }
        
    DWORD no_of_max_sleeps = secs / MAX_SLEEP_SECONDS;
            
    for(DWORD i = 0; i < no_of_max_sleeps; i++) {
        Sleep(MAX_SLEEP_SECONDS * 1000);
    }
               
    Sleep((secs % MAX_SLEEP_SECONDS) * SEC_TO_MILLIS + nano_millis);
#endif
}


#ifdef LOG4CPLUS_USE_PTHREADS
void* 
log4cplus::thread::threadStartFunc(void* arg)
#endif
#ifdef LOG4CPLUS_USE_WIN32_THREADS
DWORD WINAPI
log4cplus::thread::threadStartFunc(LPVOID arg)
#endif
{
    if(arg == NULL) {
        getLogLog().error("log4cplus::thread::threadStartFunc()- arg is NULL");
    }
    else {
        AbstractThread* thread = static_cast<AbstractThread*>(arg);
        try {
            log4cplus::helpers::SharedObjectPtr<AbstractThread> ptr(thread);
            thread->run();
        }
        catch(...) {
            // TODO --> Log
        }
        thread->running = false;
    }

#ifdef LOG4CPLUS_USE_PTHREADS
    pthread_exit(NULL);
#endif
#ifdef LOG4CPLUS_USE_WIN32_THREADS
    return NULL;
#endif
}


log4cplus::thread::AbstractThread::AbstractThread()
: running(false)
{
}



log4cplus::thread::AbstractThread::~AbstractThread()
{
}



void
log4cplus::thread::AbstractThread::start()
{
    running = true;
#ifdef LOG4CPLUS_USE_PTHREADS
    if( pthread_create(&threadId, NULL, threadStartFunc, this) ) {
        throw std::runtime_error("Thread creation was not successful");
    }
#endif
#ifdef LOG4CPLUS_USE_WIN32_THREADS
    HANDLE h = CreateThread(NULL, 0, threadStartFunc, (LPVOID)this, 0, &threadId);
    CloseHandle(h);
#endif
}

#endif // LOG4CPLUS_SINGLE_THREADED

