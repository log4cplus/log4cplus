// Module:  Log4CPLUS
// File:    stringhelper.cxx
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
// Revision 1.1  2003/04/18 22:25:30  tcsmith
// Initial version.
//

#include <log4cplus/helpers/stringhelper.h>

#include <iterator>
#include <algorithm>

#ifdef UNICODE
#  include <ccwtype>
#else
#  include <cctype>
#endif


//////////////////////////////////////////////////////////////////////////////
// Global Methods
//////////////////////////////////////////////////////////////////////////////

log4cplus::tstring
log4cplus::helpers::toupper(const log4cplus::tstring& s)
{
    tstring ret;
    std::transform(s.begin(), s.end(),
                   string_append_iterator<tstring>(ret),
#ifdef UNICODE
                   ::towupper);
#else
                   ::toupper);
#endif

    return ret;
}


log4cplus::tstring
log4cplus::helpers::tolower(const log4cplus::tstring& s)
{
    tstring ret;
    std::transform(s.begin(), s.end(),
                   string_append_iterator<tstring>(ret),
#ifdef UNICODE
                   ::towlower);
#else
                   ::tolower);
#endif

    return ret;
}


