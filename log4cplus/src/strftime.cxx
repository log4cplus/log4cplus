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
// Revision 1.1  2003/04/19 22:55:27  tcsmith
// Initial version.
//

#include <log4cplus/helpers/strftime.h>

#ifdef TM_IN_SYS_TIME
#include <sys/time.h>
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


