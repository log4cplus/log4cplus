// Module:  Log4CPLUS
// File:    thread-config.h
// Created: 4/2003
// Author:  Tad E. Smith
//
//
// Copyright 2003-2009 Tad E. Smith
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

#ifndef LOG4CPLUS_HELPERS_THREAD_CONFIG_HEADER_
#define LOG4CPLUS_HELPERS_THREAD_CONFIG_HEADER_


#ifdef LOG4CPLUS_USE_PTHREADS
#   include <pthread.h>
#   include <semaphore.h>
#   define LOG4CPLUS_MUTEX_PTR_DECLARE pthread_mutex_t*
#   define LOG4CPLUS_MUTEX_CREATE ::log4cplus::thread::createNewMutex()
#   define LOG4CPLUS_MUTEX_LOCK(mutex) pthread_mutex_lock(mutex)
#   define LOG4CPLUS_MUTEX_UNLOCK(mutex) pthread_mutex_unlock(mutex)
#   define LOG4CPLUS_MUTEX_FREE(mutex) ::log4cplus::thread::deleteMutex(mutex)
#   define LOG4CPLUS_THREAD_HANDLE_TYPE  pthread_t
#   define LOG4CPLUS_THREAD_KEY_TYPE pthread_t
#   define LOG4CPLUS_GET_CURRENT_THREAD_NAME \
    ::log4cplus::thread::getCurrentThreadName()
#   define LOG4CPLUS_GET_CURRENT_THREAD pthread_self()
#   define LOG4CPLUS_THREAD_LOCAL_TYPE pthread_key_t*
#   define LOG4CPLUS_THREAD_LOCAL_INIT(cleanup) \
    ::log4cplus::thread::createPthreadKey(cleanup)
#   define LOG4CPLUS_GET_THREAD_LOCAL_VALUE(key) pthread_getspecific(*(key))
#   define LOG4CPLUS_SET_THREAD_LOCAL_VALUE(key, value) \
    pthread_setspecific(*(key), value)
#   define LOG4CPLUS_THREAD_LOCAL_CLEANUP(key) \
    do { pthread_key_t * pthkey (key); pthread_key_delete(*pthkey); \
    delete pthkey; } while(0)
namespace log4cplus {
    namespace thread {
        LOG4CPLUS_EXPORT LOG4CPLUS_MUTEX_PTR_DECLARE createNewMutex();
        LOG4CPLUS_EXPORT void deleteMutex(LOG4CPLUS_MUTEX_PTR_DECLARE);
        LOG4CPLUS_EXPORT LOG4CPLUS_THREAD_LOCAL_TYPE createPthreadKey(void (*) (void *));
    }
}

#elif defined(LOG4CPLUS_USE_WIN32_THREADS)
#   undef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>

#   define LOG4CPLUS_MUTEX_PTR_DECLARE CRITICAL_SECTION*
#   define LOG4CPLUS_MUTEX_CREATE ::log4cplus::thread::createNewMutex()
#   define LOG4CPLUS_MUTEX_LOCK(mutex)  EnterCriticalSection(mutex)
#   define LOG4CPLUS_MUTEX_UNLOCK(mutex)  LeaveCriticalSection(mutex)
#   define LOG4CPLUS_MUTEX_FREE(mutex) ::log4cplus::thread::deleteMutex(mutex)

#   define LOG4CPLUS_THREAD_HANDLE_TYPE  HANDLE
#   define LOG4CPLUS_THREAD_KEY_TYPE  DWORD
#   define LOG4CPLUS_GET_CURRENT_THREAD  GetCurrentThreadId()
#   define LOG4CPLUS_GET_CURRENT_THREAD_NAME \
    ::log4cplus::thread::getCurrentThreadName()
#   define LOG4CPLUS_THREAD_LOCAL_TYPE DWORD
#   define LOG4CPLUS_THREAD_LOCAL_INIT(cleanup) TlsAlloc()
#   define LOG4CPLUS_GET_THREAD_LOCAL_VALUE(key) TlsGetValue(key)
#   define LOG4CPLUS_SET_THREAD_LOCAL_VALUE(key, value) \
    TlsSetValue(key, static_cast<LPVOID>(value))
#   define LOG4CPLUS_THREAD_LOCAL_CLEANUP(key) TlsFree(key)
#   if defined (_MSC_VER)
#     undef LOG4CPLUS_HAVE_TLS_SUPPORT
#     undef LOG4CPLUS_THREAD_LOCAL_VAR
// Comment out the following two lines if you do intend to use log4cplus.dll
// for loading using LoadLibrary(). The __declspec(thread) functionality is not
// compatible with such DLL use. For more information why is this necessary see
// <http://msdn.microsoft.com/en-us/library/2s9wt68x(vs.80).aspx>.
#     define LOG4CPLUS_HAVE_TLS_SUPPORT 1
#     define LOG4CPLUS_THREAD_LOCAL_VAR __declspec(thread)
#   endif


namespace log4cplus { namespace thread {

LOG4CPLUS_EXPORT LOG4CPLUS_MUTEX_PTR_DECLARE createNewMutex();
LOG4CPLUS_EXPORT void deleteMutex(LOG4CPLUS_MUTEX_PTR_DECLARE);
    
} } // namespace log4cplus { namespace thread {

#elif defined(LOG4CPLUS_SINGLE_THREADED)
#   define LOG4CPLUS_MUTEX_PTR_DECLARE int*
#   define LOG4CPLUS_MUTEX_CREATE NULL
#   define LOG4CPLUS_MUTEX_LOCK(mutex)
#   define LOG4CPLUS_MUTEX_UNLOCK(mutex)
#   define LOG4CPLUS_MUTEX_FREE(mutex)
#   define LOG4CPLUS_THREAD_HANDLE_TYPE  void *
#   define LOG4CPLUS_THREAD_KEY_TYPE int
#   define LOG4CPLUS_GET_CURRENT_THREAD 1
#   define LOG4CPLUS_GET_CURRENT_THREAD_NAME \
    LOG4CPLUS_C_STR_TO_TSTRING("single")
#   define LOG4CPLUS_THREAD_LOCAL_TYPE void*
#   define LOG4CPLUS_THREAD_LOCAL_INIT(cleanup) NULL
#   define LOG4CPLUS_GET_THREAD_LOCAL_VALUE(key) (key)
#   define LOG4CPLUS_SET_THREAD_LOCAL_VALUE(key, value) \
    do { (key) = (value); } while (0)
#   define LOG4CPLUS_THREAD_LOCAL_CLEANUP(key) do { (key) = NULL; } while (0)
#   undef LOG4CPLUS_HAVE_TLS_SUPPORT
#   undef LOG4CPLUS_THREAD_LOCAL_VAR

#else
#   error "You Must define a Threading model"
#endif

/**
 * @def LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX(mutex)
 * Begin a block synchronized on a mutex.
 *
 * @see LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
 *
 * @def LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
 * End a block synchronized on a mutex.
 *
 * @see LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX
 */
#ifndef LOG4CPLUS_SINGLE_THREADED
#  define LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX(mutex) \
             do { ::log4cplus::thread::Guard _sync_guard_object(mutex);
#else
#  define LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX(mutex) do { (void)(mutex);
#endif

#define LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX } while (0)


#endif // LOG4CPLUS_HELPERS_THREAD_CONFIG_HEADER_

