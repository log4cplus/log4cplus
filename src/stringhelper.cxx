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
// Revision 1.9  2003/06/13 17:51:37  tcsmith
// Changed to use the old style C headers.
//
// Revision 1.8  2003/06/12 23:47:40  tcsmith
// Renamed the tolower and toupper methods to avoid a name collision with a
// macro with the DEC CXX compiler.
//
// Revision 1.7  2003/05/04 07:10:48  tcsmith
// Corrected the tostring() and towstring() methods.
//
// Revision 1.6  2003/05/01 19:21:16  tcsmith
// Corrected the tostring() and towstring() methods.
//
// Revision 1.5  2003/04/19 23:31:23  tcsmith
// Added operator <<(log4cplus::tostream& stream, const char* str)
//
// Revision 1.4  2003/04/19 22:53:48  tcsmith
// Removed CRLFs.
//
// Revision 1.3  2003/04/19 22:52:21  tcsmith
// Added tostring and towstring method implementations.
//
// Revision 1.2  2003/04/19 21:33:10  tcsmith
// Replaced use of back_insert_iterator with string_append_iterator.
//
// Revision 1.1  2003/04/18 22:25:30  tcsmith
// Initial version.
//

#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/streams.h>

#include <iterator>
#include <algorithm>

#ifdef UNICODE
#  include <cwctype>
#else
#  include <ctype.h>
#endif

using namespace log4cplus;



//////////////////////////////////////////////////////////////////////////////
// Global Methods
//////////////////////////////////////////////////////////////////////////////

#ifdef UNICODE
log4cplus::tostream& 
operator <<(log4cplus::tostream& stream, const char* str)
{
    return (stream << log4cplus::helpers::towstring(str));
}


std::string 
log4cplus::helpers::tostring(const std::wstring& src)
{
    std::string ret;
    ret.resize(src.size());
    for (unsigned int i=0; i<src.size(); i++) {
        ret[i] = src[i] < 256 ? src[i] : ' ';
    }

    return ret;
}


std::wstring 
log4cplus::helpers::towstring(const std::string& src)
{
    std::wstring ret;
    ret.resize(src.size());
    for (unsigned int i=0; i<src.size(); i++) {
        ret[i] = static_cast<tchar>(src[i]);
    }

    return ret;
}
#endif



log4cplus::tstring
log4cplus::helpers::toUpper(const log4cplus::tstring& s)
{
    tstring ret;
    std::transform(s.begin(), s.end(),
                   string_append_iterator<tstring>(ret),
#ifdef UNICODE
#  if (defined(__MWERKS__) && defined(__MACOS__))
                   std::towupper);
#  else
                   ::towupper);
#  endif
#else
                   ::toupper);
#endif

    return ret;
}


log4cplus::tstring
log4cplus::helpers::toLower(const log4cplus::tstring& s)
{
    tstring ret;
    std::transform(s.begin(), s.end(),
                   string_append_iterator<tstring>(ret),
#ifdef UNICODE
#  if (defined(__MWERKS__) && defined(__MACOS__))
                   std::towlower);
#  else
                   ::towlower);
#  endif
#else
                   ::tolower);
#endif

    return ret;
}


