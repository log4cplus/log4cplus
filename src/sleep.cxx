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

#include <log4cplus/helpers/sleep.h>
#include <errno.h>
#include <time.h>

using namespace log4cplus;


///////////////////////////////////////////////////////////////////////////////
// public methods
///////////////////////////////////////////////////////////////////////////////

#define MILLIS_TO_NANOS 1000000
#define SEC_TO_MILLIS 1000
#define MAX_SLEEP_SECONDS (DWORD)4294966        // (2**32-2)/1000

void
log4cplus::helpers::sleep(unsigned long secs, unsigned long nanosecs)
{
#if defined(WIN32)
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
#else
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
}


