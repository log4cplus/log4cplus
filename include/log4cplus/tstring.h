// -*- C++ -*-
// Module:  Log4CPLUS
// File:    tstring.h
// Created: 4/2003
// Author:  Tad E. Smith
//
//
// Copyright 2003-2017 Tad E. Smith
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

/** @file */

#ifndef LOG4CPLUS_TSTRING_HEADER_
#define LOG4CPLUS_TSTRING_HEADER_

#include <log4cplus/config.hxx>

#if defined (LOG4CPLUS_HAVE_PRAGMA_ONCE)
#pragma once
#endif

#include <type_traits>
#include <string>
#include <string_view>
#include <log4cplus/tchar.h>

namespace log4cplus
{

using tstring = std::basic_string<tchar>;
using tstring_view = std::basic_string_view<tchar>;

namespace helpers
{

inline
std::string
tostring (char const * str)
{
    return std::string (str);
}

template <typename SV>
inline
std::enable_if_t<
    std::is_convertible_v<SV const &, std::string_view>
    && std::is_convertible_v<SV const &, char const *>,
    std::string>
tostring (SV const & sv)
{
    return std::string (sv);
}

inline
std::string
tostring (std::string const & str)
{
    return str;
}

inline
std::string const &
tostring (std::string & str)
{
    return str;
}

inline
std::string
tostring (std::string && str)
{
    return std::move (str);
}


inline
std::wstring
towstring (wchar_t const * str)
{
    return std::wstring (str);
}

template <typename SV>
inline
std::enable_if_t<
    std::is_convertible_v<SV const &, std::wstring_view>
    && std::is_convertible_v<SV const &, wchar_t const *>,
    std::wstring>
towstring (SV const & sv)
{
    return std::wstring (sv);
}

inline
std::wstring
towstring (std::wstring const & str)
{
    return str;
}

inline
std::wstring const &
towstring (std::wstring & str)
{
    return str;
}

inline
std::wstring
towstring (std::wstring && str)
{
    return std::move (str);
}


LOG4CPLUS_EXPORT std::string tostring(const std::wstring&);
LOG4CPLUS_EXPORT std::string tostring(const std::wstring_view&);
LOG4CPLUS_EXPORT std::string tostring(wchar_t const *);

LOG4CPLUS_EXPORT std::wstring towstring(const std::string&);
LOG4CPLUS_EXPORT std::wstring towstring(const std::string_view&);
LOG4CPLUS_EXPORT std::wstring towstring(char const *);

} // namespace helpers

#ifdef UNICODE

#define LOG4CPLUS_C_STR_TO_TSTRING(STRING) log4cplus::helpers::towstring(STRING)
#define LOG4CPLUS_STRING_TO_TSTRING(STRING) log4cplus::helpers::towstring(STRING)
#define LOG4CPLUS_TSTRING_TO_STRING(STRING) log4cplus::helpers::tostring(STRING)

#else // UNICODE

#define LOG4CPLUS_C_STR_TO_TSTRING(STRING) (std::string(STRING))
#define LOG4CPLUS_STRING_TO_TSTRING(STRING) STRING
#define LOG4CPLUS_TSTRING_TO_STRING(STRING) STRING

#endif // UNICODE

} // namespace log4cplus


#endif // LOG4CPLUS_TSTRING_HEADER_
