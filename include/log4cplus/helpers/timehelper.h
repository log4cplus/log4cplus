// Module:  Log4CPLUS
// File:    timehelper.h
// Created: 6/2003
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

#ifndef _LOG4CPLUS_HELPERS_TIME_HELPER_HEADER_
#define _LOG4CPLUS_HELPERS_TIME_HELPER_HEADER_

#include <log4cplus/config.h>
#include <log4cplus/tstring.h>

#ifdef TM_IN_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif


#if defined(HAVE_GMTIME_R) && !defined(LOG4CPLUS_SINGLE_THREADED)
#define LOG4CPLUS_NEED_GMTIME_R
#endif

#if defined(HAVE_LOCALTIME_R) && !defined(LOG4CPLUS_SINGLE_THREADED)
#define LOG4CPLUS_NEED_LOCALTIME_R
#endif



namespace log4cplus {
    namespace helpers {
        LOG4CPLUS_EXPORT size_t strftime(tchar* s, size_t max, 
                                         const tchar* format, const struct tm* tm);

        LOG4CPLUS_EXPORT struct tm *gmtime(const time_t *clock, struct tm *res);

        LOG4CPLUS_EXPORT struct tm *localtime(const time_t *clock, struct tm *res);
    }
}


#endif // _LOG4CPLUS_HELPERS_TIME_HELPER_HEADER_

