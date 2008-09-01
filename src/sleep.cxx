// Module:  Log4CPLUS
// File:    sleep.cxx
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

#include <log4cplus/helpers/sleep.h>
#include <log4cplus/helpers/timehelper.h>

#include <errno.h>

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
#if defined(_WIN32)
    DWORD nano_millis = nanosecs / static_cast<unsigned long>(MILLIS_TO_NANOS);
    if (secs <= MAX_SLEEP_SECONDS) {
        Sleep((secs * SEC_TO_MILLIS) + nano_millis);
        return;
    }
        
    DWORD no_of_max_sleeps = secs / MAX_SLEEP_SECONDS;
            
    for(DWORD i = 0; i < no_of_max_sleeps; i++) {
        Sleep(MAX_SLEEP_SECONDS * SEC_TO_MILLIS);
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


void
log4cplus::helpers::sleepmillis(unsigned long millis)
{
    unsigned long secs = millis / SEC_TO_MILLIS;
    unsigned long nanosecs = (millis % SEC_TO_MILLIS) * MILLIS_TO_NANOS;
    sleep(secs, nanosecs);
}

