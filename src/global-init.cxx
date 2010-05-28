// Module:  Log4CPLUS
// File:    global-init.cxx
// Created: 5/2003
// Author:  Tad E. Smith
//
//
// Copyright 2003-2010 Tad E. Smith
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

#include <log4cplus/config.hxx>
#include <log4cplus/logger.h>
#include <log4cplus/ndc.h>
#include <log4cplus/helpers/loglog.h>


// Forward Declarations
namespace log4cplus
{


void initializeFactoryRegistry();


void initializeLog4cplus()
{
    static bool initialized = false;
    if (initialized)
        return;

    helpers::LogLog::getLogLog();
    getLogLevelManager ();
    getNDC();
    Logger::getRoot();
    initializeFactoryRegistry();

    initialized = true;
}


} // namespace log4cplus


#if defined (_WIN32) && defined (LOG4CPLUS_BUILD_DLL)

BOOL WINAPI DllMain(LOG4CPLUS_DLLMAIN_HINSTANCE hinstDLL,  // handle to DLL module
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
 
#else

namespace {

    class _static_log4cplus_initializer {
    public:
        _static_log4cplus_initializer() {
            log4cplus::initializeLog4cplus();
        }
    } static initializer;
}

#endif
