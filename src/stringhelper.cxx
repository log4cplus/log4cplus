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
        ret[i] = (src[i] < 256 ? static_cast<char>(src[i]) : ' ');
    }

    return ret;
}


std::wstring 
log4cplus::helpers::towstring(const std::string& src)
{
    std::wstring ret;
    ret.resize(src.size());
    for (unsigned int i=0; i<src.size(); i++) {
      ret[i] = static_cast<tchar>
        (static_cast<unsigned char> (src[i]));
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


