// Module:  Log4CPLUS
// File:    strftime.cxx
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
// $Log: not supported by cvs2svn $
// Revision 1.2  2003/04/19 23:51:17  tcsmith
// Now call the strftime() function in the global namespace.
//
// Revision 1.1  2003/04/19 22:55:27  tcsmith
// Initial version.
//

#include <log4cplus/helpers/timehelper.h>

#if defined(HAVE_GMTIME_R) && !defined(LOG4CPLUS_SINGLE_THREADED)
#define LOG4CPLUS_NEED_GMTIME_R
#endif

#if defined(HAVE_LOCALTIME_R) && !defined(LOG4CPLUS_SINGLE_THREADED)
#define LOG4CPLUS_NEED_LOCALTIME_R
#endif


using namespace log4cplus;



//////////////////////////////////////////////////////////////////////////////
// Global methods
//////////////////////////////////////////////////////////////////////////////

size_t
log4cplus::helpers::strftime(tchar* s, size_t max, 
                             const tchar* format, const struct tm* tm)
{
#ifdef UNICODE
    return ::wcsftime(s, max, format, tm);
#else
    return ::strftime(s, max, format, tm);
#endif
}



struct tm*
log4cplus::helpers::gmtime(const time_t *clock, struct tm *res)
{
#ifdef LOG4CPLUS_NEED_GMTIME_R
    return ::gmtime_r(clock, res);
#else
    res = res;  // remove compiler warning
    return ::gmtime(clock);
#endif
}



struct tm*
log4cplus::helpers::localtime(const time_t *clock, struct tm *res)
{
#ifdef LOG4CPLUS_NEED_LOCALTIME_R
    return ::localtime_r(clock, res);
#else
    res = res;  // remove compiler warning
    return ::localtime(clock);
#endif
}



