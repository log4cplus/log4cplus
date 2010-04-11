// Module:  Log4CPLUS
// File:    tstring.h
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

/** @file */

#ifndef LOG4CPLUS_TSTRING_HEADER_
#define LOG4CPLUS_TSTRING_HEADER_

#include <log4cplus/config.h>
#include <string>

#ifdef UNICODE
#  define LOG4CPLUS_TEXT(STRING) L##STRING
#else
#  define LOG4CPLUS_TEXT(STRING) STRING
#endif // UNICODE


#ifdef UNICODE
namespace log4cplus {
    typedef wchar_t tchar;
    typedef std::wstring tstring;

    namespace helpers {
        LOG4CPLUS_EXPORT std::string tostring(const std::wstring&);
        LOG4CPLUS_EXPORT std::wstring towstring(const std::string&);
    }

}

#define LOG4CPLUS_C_STR_TO_TSTRING(STRING) log4cplus::helpers::towstring(STRING)
#define LOG4CPLUS_STRING_TO_TSTRING(STRING) log4cplus::helpers::towstring(STRING)
#define LOG4CPLUS_TSTRING_TO_STRING(STRING) log4cplus::helpers::tostring(STRING)

#else
namespace log4cplus {
    typedef char tchar;
    typedef std::string tstring;
}

#define LOG4CPLUS_C_STR_TO_TSTRING(STRING) std::string(STRING)
#define LOG4CPLUS_STRING_TO_TSTRING(STRING) STRING
#define LOG4CPLUS_TSTRING_TO_STRING(STRING) STRING

#endif // UNICODE

#endif // LOG4CPLUS_TSTRING_HEADER_

