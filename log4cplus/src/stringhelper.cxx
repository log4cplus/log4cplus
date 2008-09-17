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
#  include <cassert>
#  include <vector>
#  include <cwctype>
#  include <cstring>
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


#ifdef LOG4CPLUS_WORKING_LOCALE

static
void
clear_mbstate (mbstate_t & mbs)
{
    // Initialize/clear mbstate_t type.
    // XXX: This is just a hack that works. The shape of mbstate_t varies
    // from single unsigned to char[128]. Without some sort of initialization
    // the codecvt::in/out methods randomly fail because the initial state is
    // random/invalid.
    ::memset (&mbs, 0, sizeof (mbstate_t));
}


std::wstring
log4cplus::helpers::towstring(const std::string& src, std::locale const & loc)
{
    std::wstring outstr;

    typedef std::codecvt<wchar_t, char, mbstate_t> CodeCvt;
    const CodeCvt & cdcvt = std::use_facet<CodeCvt>(loc);
    mbstate_t state;
    clear_mbstate (state);

    char const * const from_first = src.c_str ();
    size_t const from_size = src.size ();
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
        else
        {
            assert (0);
            break;
        }
    }
    converted = to_next - to_first;

    outstr.assign (dest.begin (), dest.begin () + converted);
    return outstr;
}


std::string
log4cplus::helpers::tostring(const std::wstring& src, std::locale const & loc)
{
    std::string outstr;

    typedef std::codecvt<wchar_t, char, mbstate_t> CodeCvt;
    const CodeCvt & cdcvt = std::use_facet<CodeCvt>(loc);
    mbstate_t state;
    clear_mbstate (state);

    wchar_t const * const from_first = src.c_str ();
    size_t const from_size = src.size ();
    wchar_t const * const from_last = from_first + from_size;
    wchar_t const * from_next = from_first;

    std::vector<char> dest (from_size);

    char * to_first = &dest.front ();
    size_t to_size = dest.size ();
    char * to_last = to_first + to_size;
    char * to_next = to_first;

    CodeCvt::result result;
    size_t converted = 0;
    while (true)
    {
        result = cdcvt.out (
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
        else
        {
            assert (0);
            break;
        }
    }
    converted = to_next - to_first;

    outstr.assign (dest.begin (), dest.begin () + converted);
    return outstr;
}

#else // LOG4CPLUS_WORKING_LOCALE

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

#endif // LOG4CPLUS_WORKING_LOCALE

#endif // UNICODE


log4cplus::tstring
log4cplus::helpers::toUpper(const log4cplus::tstring& s)
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


