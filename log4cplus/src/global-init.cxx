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
// $Log: not supported by cvs2svn $

#include <log4cplus/config.h>
#include <log4cplus/logger.h>
#include <log4cplus/ndc.h>
#include <log4cplus/helpers/loglog.h>


// Forward Declarations
namespace log4cplus {
    void initializeFactoryRegistry();
}


namespace log4cplus {
    void initializeLog4cplus() {
        log4cplus::helpers::getLogLog();
        getNDC();
        Logger::getRoot();
        initializeFactoryRegistry();
    }
}


#if !defined(WIN32) || !defined(LOG4CPLUS_BUILD_DLL)
namespace {

    class _static_log4cplus_initializer {
    public:
        _static_log4cplus_initializer() {
            log4cplus::initializeLog4cplus();
        }
    } initializer;
}


#else /* Built as part of a WIN32 DLL */ 
BOOL WINAPI DllMain(HINSTANCE hinstDLL,  // handle to DLL module
                    DWORD fdwReason,     // reason for calling function
                    LPVOID lpReserved )  // reserved
{
    // Perform actions based on the reason for calling.
    switch( fdwReason ) 
    { 
        case DLL_PROCESS_ATTACH:
            log4cplus::initializeLog4cplus();
            break;

        case DLL_THREAD_ATTACH:
         // Do thread-specific initialization.
            break;

        case DLL_THREAD_DETACH:
         // Do thread-specific cleanup.
            break;

        case DLL_PROCESS_DETACH:
         // Perform any necessary cleanup.
            break;
    }

    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
 
#endif
