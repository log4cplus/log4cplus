// Module:  Log4CPLUS
// File:    global-init.cxx
// Created: 5/2003
// Author:  Tad E. Smith
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

#include <log4cplus/config.hxx>
#include <log4cplus/logger.h>
#include <log4cplus/ndc.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/internal/internal.h>


// Forward Declarations
namespace log4cplus
{


namespace internal
{

#if ! defined (LOG4CPLUS_SINGLE_THREADED)
#  if defined (LOG4CPLUS_THREAD_LOCAL_VAR)

LOG4CPLUS_THREAD_LOCAL_VAR per_thread_data * ptd;

#  else

LOG4CPLUS_THREAD_LOCAL_TYPE tls_storage_key;

#  endif
#endif

} // namespace internal


void initializeFactoryRegistry();


namespace
{


//! Thread local storage clean up function for POSIX threads.
static 
void 
ptd_cleanup_func (void * arg)
{
    assert (arg == internal::get_ptd ());
    (void)arg;
    threadCleanup ();
}


} // namespace


void initializeLog4cplus()
{
    static bool initialized = false;
    if (initialized)
        return;

#if ! defined (LOG4CPLUS_SINGLE_THREADED)
#  if ! defined (LOG4CPLUS_THREAD_LOCAL_VAR)

    internal::tls_storage_key = LOG4CPLUS_THREAD_LOCAL_INIT (ptd_cleanup_func);

#  endif
#endif

    helpers::LogLog::getLogLog();
    getLogLevelManager ();
    getNDC();
    Logger::getRoot();
    initializeFactoryRegistry();

    initialized = true;
}


} // namespace log4cplus


#if defined (_WIN32) && (defined (LOG4CPLUS_BUILD_DLL) || defined (log4cplus_EXPORTS))

BOOL WINAPI DllMain(HINSTANCE hinstDLL,  // handle to DLL module
                    DWORD fdwReason,     // reason for calling function
                    LPVOID lpReserved )  // reserved
{
    using namespace log4cplus;

    // Perform actions based on the reason for calling.
    switch( fdwReason ) 
    { 
    case DLL_PROCESS_ATTACH:
    {
        log4cplus::initializeLog4cplus();
#if ! defined (LOG4CPLUS_SINGLE_THREADED)
        // Do thread-specific initialization for the main thread.
        assert (! internal::get_ptd ());
        internal::per_thread_data * ptd = new internal::per_thread_data;
        internal::set_ptd (ptd);
#endif
        break;
    }

    case DLL_THREAD_ATTACH:
    {
#if ! defined (LOG4CPLUS_SINGLE_THREADED)
        assert (! internal::get_ptd ());
        // Do thread-specific initialization.
        internal::per_thread_data * ptd = new internal::per_thread_data;
        internal::set_ptd (ptd);
#endif
        break;
    }

    case DLL_THREAD_DETACH:
    {
#if ! defined (LOG4CPLUS_SINGLE_THREADED)
        // Do thread-specific cleanup.
        threadCleanup ();
#endif
        break;
    }

    case DLL_PROCESS_DETACH:
    {
        // Perform any necessary cleanup.
#if ! defined (LOG4CPLUS_SINGLE_THREADED)
        // Do thread-specific cleanup.
        threadCleanup ();
#  if ! defined (LOG4CPLUS_THREAD_LOCAL_VAR)
        LOG4CPLUS_THREAD_LOCAL_CLEANUP (internal::tls_storage_key);
#  endif
#endif
        break;
    }

    }

    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
 
#else

namespace {

    struct _static_log4cplus_initializer
    {
        _static_log4cplus_initializer ()
        {
            log4cplus::initializeLog4cplus();
        }

        ~_static_log4cplus_initializer ()
        {
            // Last thread cleanup.
            log4cplus::threadCleanup ();
        }
    } static initializer;
}


#endif


namespace log4cplus
{


void
threadCleanup ()
{
#if ! defined (LOG4CPLUS_SINGLE_THREADED)
    // Do thread-specific cleanup.
    internal::per_thread_data * ptd = internal::get_ptd ();
    delete ptd;
    internal::set_ptd (0);
#endif
}


} // namespace log4cplus
