// Module:  Log4CPLUS
// File:    stringhelper.cxx
// Created: 4/2003
// Author:  Tad E. Smith
//
//
// Copyright 2003-2010 Tad E. Smith
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

#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/streams.h>
#include <log4cplus/internal/internal.h>

#include <locale>
#include <iterator>
#include <algorithm>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <cctype>

#ifdef UNICODE
#  include <cassert>
#  include <vector>
#endif


namespace log4cplus
{

namespace internal
{

log4cplus::tstring const empty_str;

} // namespace internal

} // namespace log4cplus


//////////////////////////////////////////////////////////////////////////////
// Global Methods
//////////////////////////////////////////////////////////////////////////////

#if defined (UNICODE) && defined (LOG4CPLUS_ENABLE_GLOBAL_C_STRING_STREAM_INSERTER)

log4cplus::tostream& 
operator <<(log4cplus::tostream& stream, const char* str)
{
    return (stream << log4cplus::helpers::towstring(str));
}

#endif


namespace log4cplus
{

namespace helpers
{


void
clear_mbstate (std::mbstate_t & mbs)
{
    // Initialize/clear mbstate_t type.
    // XXX: This is just a hack that works. The shape of mbstate_t varies
    // from single unsigned to char[128]. Without some sort of initialization
    // the codecvt::in/out methods randomly fail because the initial state is
    // random/invalid.
    std::memset (&mbs, 0, sizeof (std::mbstate_t));
}


#if defined (UNICODE)
#if defined (LOG4CPLUS_POOR_MANS_CHCONV)

static
void
tostring_internal (std::string & ret, wchar_t const * src, std::size_t size)
{
    ret.resize(size);
    for (std::size_t i = 0; i < size; ++i)
    {
        ret[i] = static_cast<unsigned> (static_cast<int> (src[i])) < 256
            ? static_cast<char>(src[i]) : '?';
    }
}


std::string 
tostring(const std::wstring& src)
{
    std::string ret;
    tostring_internal (ret, src.c_str (), src.size ());
    return ret;
}


std::string 
tostring(wchar_t const * src)
{
    assert (src);
    std::string ret;
    tostring_internal (ret, src, std::wcslen (src));
    return ret;
}


static
void
towstring_internal (std::wstring & ret, char const * src, std::size_t size)
{
    ret.resize(size);
    for (std::size_t i = 0; i < size; ++i)
    {
        ret[i] = static_cast<wchar_t>
            (static_cast<unsigned char> (src[i]));
    }
}


std::wstring 
towstring(const std::string& src)
{
    std::wstring ret;
    towstring_internal (ret, src.c_str (), src.size ());
    return ret;
}


std::wstring 
towstring(char const * src)
{
    assert (src);
    std::wstring ret;
    towstring_internal (ret, src, std::strlen (src));
    return ret;
}

#endif // LOG4CPLUS_WORKING_LOCALE

#endif // UNICODE


namespace
{


struct toupper_func
{
    tchar
    operator () (tchar ch) const
    {
#ifdef UNICODE
        return std::towupper (ch);
#else
        return std::toupper (static_cast<unsigned char>(ch));
#endif
    }
};


struct tolower_func
{
    tchar
    operator () (tchar ch) const
    {
#ifdef UNICODE
        return std::towlower (ch);
#else
        return std::tolower (static_cast<unsigned char>(ch));
#endif
    }
};


} // namespace


tstring
toUpper(const tstring& s)
{
    tstring ret;
    std::transform(s.begin(), s.end(), std::back_inserter (ret),
        toupper_func ());
    return ret;
}


tstring
toLower(const tstring& s)
{
    tstring ret;
    std::transform(s.begin(), s.end(), std::back_inserter (ret),
        tolower_func ());
    return ret;
}


} // namespace helpers

} // namespace log4cplus
