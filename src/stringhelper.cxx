// Module:  Log4CPLUS
// File:    stringhelper.cxx
// Created: 4/2003
// Author:  Tad E. Smith
//
//
// Copyright 2003-2009 Tad E. Smith
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

#ifdef UNICODE

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

#ifdef UNICODE

#ifdef LOG4CPLUS_WORKING_LOCALE

static
void
clear_mbstate (std::mbstate_t & mbs)
{
    // Initialize/clear mbstate_t type.
    // XXX: This is just a hack that works. The shape of mbstate_t varies
    // from single unsigned to char[128]. Without some sort of initialization
    // the codecvt::in/out methods randomly fail because the initial state is
    // random/invalid.
    ::memset (&mbs, 0, sizeof (std::mbstate_t));
}


static
void
towstring_internal (std::wstring & outstr, const char * src, size_t size,
    std::locale const & loc)
{
    if (size == 0)
    {
        outstr.clear ();
        return;
    }

    typedef std::codecvt<wchar_t, char, std::mbstate_t> CodeCvt;
    const CodeCvt & cdcvt = std::use_facet<CodeCvt>(loc);
    std::mbstate_t state;
    clear_mbstate (state);

    char const * from_first = src;
    size_t const from_size = size;
    char const * const from_last = from_first + from_size;
    char const * from_next = from_first;

    std::vector<wchar_t> dest (from_size);

    wchar_t * to_first = &dest.front ();
    size_t to_size = dest.size ();
    wchar_t * to_last = to_first + to_size;
    wchar_t * to_next = to_first;

    CodeCvt::result result;
    size_t converted = 0;
    while (true)
    {
        result = cdcvt.in (
            state, from_first, from_last,
            from_next, to_first, to_last,
            to_next);
        // XXX: Even if only half of the input has been converted the
        // in() method returns CodeCvt::ok. I think it should return
        // CodeCvt::partial.
        if ((result == CodeCvt::partial || result == CodeCvt::ok)
            && from_next != from_last)
        {
            to_size = dest.size () * 2;
            dest.resize (to_size);
            converted = to_next - to_first;
            to_first = &dest.front ();
            to_last = to_first + to_size;
            to_next = to_first + converted;
            continue;
        }
        else if (result == CodeCvt::ok && from_next == from_last)
            break;
        else if (result == CodeCvt::error
            && to_next != to_last && from_next != from_last)
        {
            clear_mbstate (state);
            ++from_next;
            from_first = from_next;
            *to_next = L'?';
            ++to_next;
            to_first = to_next;
        }
        else
            break;
    }
    converted = to_next - &dest[0];

    outstr.assign (dest.begin (), dest.begin () + converted);
}


std::wstring 
towstring(const std::string& src, std::locale const & loc)
{
    std::wstring ret;
    towstring_internal (ret, src.c_str (), src.size (), loc);
    return ret;
}


std::wstring 
towstring(char const * src, std::locale const & loc)
{
    std::wstring ret;
    towstring_internal (ret, src, std::strlen (src), loc);
    return ret;
}


static
void
tostring_internal (std::string & outstr, const wchar_t * src, size_t size,
    std::locale const & loc)
{
    if (size == 0)
    {
        outstr.clear ();
        return;
    }

    typedef std::codecvt<wchar_t, char, std::mbstate_t> CodeCvt;
    const CodeCvt & cdcvt = std::use_facet<CodeCvt>(loc);
    std::mbstate_t state;
    clear_mbstate (state);

    wchar_t const * from_first = src;
    size_t const from_size = size;
    wchar_t const * const from_last = from_first + from_size;
    wchar_t const * from_next = from_first;

    std::vector<char> dest (from_size);

    char * to_first = &dest.front ();
    size_t to_size = dest.size ();
    char * to_last = to_first + to_size;
    char * to_next = to_first;

    CodeCvt::result result;
    size_t converted = 0;
    while (from_next != from_last)
    {
        result = cdcvt.out (
            state, from_first, from_last,
            from_next, to_first, to_last,
            to_next);
        // XXX: Even if only half of the input has been converted the
        // in() method returns CodeCvt::ok with VC8. I think it should
        // return CodeCvt::partial.
        if ((result == CodeCvt::partial || result == CodeCvt::ok)
            && from_next != from_last)
        {
            to_size = dest.size () * 2;
            dest.resize (to_size);
            converted = to_next - to_first;
            to_first = &dest.front ();
            to_last = to_first + to_size;
            to_next = to_first + converted;
        }
        else if (result == CodeCvt::ok && from_next == from_last)
            break;
        else if (result == CodeCvt::error
            && to_next != to_last && from_next != from_last)
        {
            clear_mbstate (state);
            ++from_next;
            from_first = from_next;
            *to_next = '?';
            ++to_next;
            to_first = to_next;
        }
        else
            break;
    }
    converted = to_next - &dest[0];

    outstr.assign (dest.begin (), dest.begin () + converted);
}


std::string 
tostring(const std::wstring& src, std::locale const & loc)
{
    std::string ret;
    tostring_internal (ret, src.c_str (), src.size (), loc);
    return ret;
}


std::string 
tostring(wchar_t const * src, std::locale const & loc)
{
    std::string ret;
    tostring_internal (ret, src, std::wcslen (src), loc);
    return ret;
}


#else // LOG4CPLUS_WORKING_LOCALE


static
void
tostring_internal (std::string & ret, wchar_t const * src, size_t size)
{
    ret.resize(size);
    for (size_t i = 0; i < size; ++i)
    {
        ret[i] = static_cast<unsigned> (static_cast<int> (src[i])) < 256
            ? static_cast<char>(src[i]) : ' ';
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
towstring_internal (std::wstring & ret, char const * src, size_t size)
{
    ret.resize(size);
    for (size_t i = 0; i < size; ++i)
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


tstring
toUpper(const log4cplus::tstring& s)
{
    tstring ret;
    std::transform(s.begin(), s.end(),
                   string_append_iterator<tstring>(ret),
#ifdef UNICODE
#  if (defined(__MWERKS__) && defined(__MACOS__)) || defined (LOG4CPLUS_WORKING_LOCALE)
                   std::towupper);
#  else
                   ::towupper);
#  endif
#else
                   ::toupper);
#endif

    return ret;
}


tstring
toLower(const tstring& s)
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


} // namespace helpers

} // namespace log4cplus
