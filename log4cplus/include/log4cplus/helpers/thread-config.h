// Module:  Log4CPLUS
// File:    thread-config.h
// Created: 4/2003
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

#ifndef LOG4CPLUS_HELPERS_THREAD_CONFIG_HEADER_
#define LOG4CPLUS_HELPERS_THREAD_CONFIG_HEADER_


#ifdef LOG4CPLUS_USE_PTHREADS
#   include <pthread.h>
#   define LOG4CPLUS_MUTEX_PTR_DECLARE pthread_mutex_t*
#   define LOG4CPLUS_MUTEX_CREATE ::log4cplus::thread::createNewMutex()
#   define LOG4CPLUS_MUTEX_ASSIGN( mutex_a, mutex_b) mutex_a = mutex_b
#   define LOG4CPLUS_MUTEX_LOCK( mutex ) pthread_mutex_lock(mutex)
#   define LOG4CPLUS_MUTEX_UNLOCK( mutex ) pthread_mutex_unlock(mutex)
#   define LOG4CPLUS_MUTEX_FREE(mutex) ::log4cplus::thread::deleteMutex(mutex)
#   define LOG4CPLUS_THREAD_KEY_TYPE pthread_t
#   define LOG4CPLUS_GET_CURRENT_THREAD_NAME ::log4cplus::thread::getCurrentThreadName()

#   define LOG4CPLUS_GET_CURRENT_THREAD pthread_self()
#   define LOG4CPLUS_THREAD_LOCAL_TYPE pthread_key_t*
#   define LOG4CPLUS_THREAD_LOCAL_INIT ::log4cplus::thread::createPthreadKey()
#   define LOG4CPLUS_GET_THREAD_LOCAL_VALUE( key ) pthread_getspecific(*key)
#   define LOG4CPLUS_SET_THREAD_LOCAL_VALUE( key, value ) pthread_setspecific(*key, value)
#   define LOG4CPLUS_THREAD_LOCAL_CLEANUP( key ) pthread_key_delete(*key)
namespace log4cplus {
    namespace thread {
        LOG4CPLUS_MUTEX_PTR_DECLARE createNewMutex();
        void deleteMutex(LOG4CPLUS_MUTEX_PTR_DECLARE);
        LOG4CPLUS_THREAD_LOCAL_TYPE createPthreadKey();
    }
}

#elif defined(LOG4CPLUS_USE_WIN32_THREADS)
#   include <windows.h>
#   define LOG4CPLUS_MUTEX_PTR_DECLARE CRITICAL_SECTION*
#   define LOG4CPLUS_MUTEX_CREATE ::log4cplus::thread::createNewMutex()
#   define LOG4CPLUS_MUTEX_ASSIGN( mutex_a, mutex_b) mutex_a = mutex_b
#   define LOG4CPLUS_MUTEX_LOCK( mutex )  EnterCriticalSection(mutex)
#   define LOG4CPLUS_MUTEX_UNLOCK( mutex )  LeaveCriticalSection(mutex)
#   define LOG4CPLUS_MUTEX_FREE(mutex) ::log4cplus::thread::deleteMutex(mutex)
#   define LOG4CPLUS_THREAD_KEY_TYPE  DWORD
#   define LOG4CPLUS_GET_CURRENT_THREAD  GetCurrentThreadId()
#   define LOG4CPLUS_GET_CURRENT_THREAD_NAME ::log4cplus::thread::getCurrentThreadName()
#   define LOG4CPLUS_THREAD_LOCAL_TYPE DWORD
#   define LOG4CPLUS_THREAD_LOCAL_INIT TlsAlloc()
#   define LOG4CPLUS_GET_THREAD_LOCAL_VALUE( key ) TlsGetValue(key)
#   define LOG4CPLUS_SET_THREAD_LOCAL_VALUE( key, value ) \
       TlsSetValue(key, static_cast<LPVOID>(value))
#   define LOG4CPLUS_THREAD_LOCAL_CLEANUP( key ) TlsFree(key)
namespace log4cplus {
    namespace thread {
        LOG4CPLUS_EXPORT LOG4CPLUS_MUTEX_PTR_DECLARE createNewMutex();
        LOG4CPLUS_EXPORT void deleteMutex(LOG4CPLUS_MUTEX_PTR_DECLARE);
    }
}

#elif defined(LOG4CPLUS_SINGLE_THREADED)
#   define LOG4CPLUS_MUTEX_PTR_DECLARE int*
#   define LOG4CPLUS_MUTEX_CREATE NULL
#   define LOG4CPLUS_MUTEX_ASSIGN( mutex_a, mutex_b) mutex_a = mutex_b;
#   define LOG4CPLUS_MUTEX_LOCK( mutex )
#   define LOG4CPLUS_MUTEX_UNLOCK( mutex )
#   define LOG4CPLUS_MUTEX_FREE(mutex)
#   define LOG4CPLUS_THREAD_KEY_TYPE int
#   define LOG4CPLUS_GET_CURRENT_THREAD (int)1
#   define LOG4CPLUS_GET_CURRENT_THREAD_NAME "single"
#   define LOG4CPLUS_THREAD_LOCAL_TYPE void*
#   define LOG4CPLUS_THREAD_LOCAL_INIT NULL
#   define LOG4CPLUS_GET_THREAD_LOCAL_VALUE( key ) key
#   define LOG4CPLUS_SET_THREAD_LOCAL_VALUE( key, value ) key = value;
#   define LOG4CPLUS_THREAD_LOCAL_CLEANUP( key ) key = NULL

#else
#   error "You Must define a Threading model"
#endif

/**
 * @def BEGIN_SYNCHRONIZE_ON_MUTEX(mutex)
 * Begin a block synchronized on a mutex.
 *
 * @see END_SYNCHRONIZE_ON_MUTEX
 *
 * @def END_SYNCHRONIZE_ON_MUTEX
 * End a block synchronized on a mutex.
 *
 * @see BEGIN_SYNCHRONIZE_ON_MUTEX
 */
#ifndef LOG4CPLUS_SINGLE_THREADED
#  define LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex ) \
             {::log4cplus::thread::Guard _sync_guard_object(mutex);
#else
#  define LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex ) {
#endif

#define LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX }


#endif // LOG4CPLUS_HELPERS_THREAD_CONFIG_HEADER_

