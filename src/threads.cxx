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
// $Log: not supported by cvs2svn $
// Revision 1.9  2003/05/22 21:17:32  tcsmith
// Moved the sleep() method into sleep.cxx
//
// Revision 1.8  2003/04/29 17:38:45  tcsmith
// Added "return NULL" to the threadStartFunc() method to make the Sun Forte
// compiler happy.
//
// Revision 1.7  2003/04/18 21:58:47  tcsmith
// Converted from std::string to log4cplus::tstring.
//
// Revision 1.6  2003/04/03 01:31:43  tcsmith
// Standardized the formatting.
//

#ifndef LOG4CPLUS_SINGLE_THREADED

#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/threads.h>
#include <log4cplus/helpers/timehelper.h>

#include <exception>
#include <sstream>
#include <stdexcept>
#include <errno.h>

#ifdef LOG4CPLUS_USE_PTHREADS
#    include <sched.h>
#endif

using namespace log4cplus::helpers;


///////////////////////////////////////////////////////////////////////////////
// public methods
///////////////////////////////////////////////////////////////////////////////

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
        getLogLog().error(LOG4CPLUS_TEXT("log4cplus::thread::threadStartFunc()- arg is NULL"));
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
    return NULL;
#endif
#ifdef LOG4CPLUS_USE_WIN32_THREADS
    return NULL;
#endif
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

