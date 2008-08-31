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
// Revision 1.6.2.1  2008/08/31 11:26:55  wilx
// Mark anonymous namespace symbols static so that they do not get
// exported with ugly names in DLL.
//
// Revision 1.6  2003/08/22 06:54:18  tcsmith
// Now set the initialized flag in the initializeLog4cplus() method.
//
// Revision 1.5  2003/08/08 05:36:51  tcsmith
// Changed the #if checks to look for _WIN32 and not WIN32.
//
// Revision 1.4  2003/06/29 16:48:24  tcsmith
// Modified to support that move of the getLogLog() method into the LogLog
// class.
//
// Revision 1.3  2003/06/03 20:25:19  tcsmith
// Modified initializeLog4cplus() so that it can safely be called multiple
// times.
//
// Revision 1.2  2003/05/04 08:41:33  tcsmith
// Formatting cleanup.
//
// Revision 1.1  2003/05/04 07:25:16  tcsmith
// Initial version.
//

#include <log4cplus/config.hxx>
#include <log4cplus/logger.h>
#include <log4cplus/ndc.h>
#include <log4cplus/helpers/loglog.h>


// Forward Declarations
namespace log4cplus {
    void initializeFactoryRegistry();
}


namespace log4cplus {
    void initializeLog4cplus() {
        static bool initialized = false;
        if(!initialized) {
            log4cplus::helpers::LogLog::getLogLog();
            getNDC();
            Logger::getRoot();
            initializeFactoryRegistry();
            initialized = true;
        }
    }
}


#if !defined(_WIN32) || !defined(LOG4CPLUS_BUILD_DLL)
namespace {

    class _static_log4cplus_initializer {
    public:
        _static_log4cplus_initializer() {
            log4cplus::initializeLog4cplus();
        }
    } static initializer;
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


