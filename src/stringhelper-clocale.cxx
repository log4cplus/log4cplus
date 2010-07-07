//   Copyright (C) 2010, Vaclav Haisman. All rights reserved.
//   
//   Redistribution and use in source and binary forms, with or without modifica-
//   tion, are permitted provided that the following conditions are met:
//   
//   1. Redistributions of  source code must  retain the above copyright  notice,
//      this list of conditions and the following disclaimer.
//   
//   2. Redistributions in binary form must reproduce the above copyright notice,
//      this list of conditions and the following disclaimer in the documentation
//      and/or other materials provided with the distribution.
//   
//   THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED WARRANTIES,
//   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//   FITNESS  FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN NO  EVENT SHALL  THE
//   APACHE SOFTWARE  FOUNDATION  OR ITS CONTRIBUTORS  BE LIABLE FOR  ANY DIRECT,
//   INDIRECT, INCIDENTAL, SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLU-
//   DING, BUT NOT LIMITED TO, PROCUREMENT  OF SUBSTITUTE GOODS OR SERVICES; LOSS
//   OF USE, DATA, OR  PROFITS; OR BUSINESS  INTERRUPTION)  HOWEVER CAUSED AND ON
//   ANY  THEORY OF LIABILITY,  WHETHER  IN CONTRACT,  STRICT LIABILITY,  OR TORT
//   (INCLUDING  NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT OF THE  USE OF
//   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/helpers/loglog.h>

#ifdef LOG4CPLUS_HAVE_LIMITS_H
#include <limits.h>
#endif

#ifdef LOG4CPLUS_HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef LOG4CPLUS_HAVE_WCHAR_H
#include <wchar.h>
#endif

#include <cstring>
#include <cwchar>
#include <cassert>


namespace log4cplus
{

namespace helpers
{

#if defined (UNICODE)
#if defined (LOG4CPLUS_WORKING_C_LOCALE)

static
void
tostring_internal (std::string & result, wchar_t const * src, std::size_t size)
{
    std::size_t result_size = size + size / 3 + 1;
    std::size_t ret;

    result.resize (result_size);
    while ((ret = wcstombs (&result[0], src, result_size)) == result_size)
    {
        result_size *= 2;
        result.resize (result_size);
    }

    if (ret != static_cast<std::size_t>(-1))
        result.resize (ret);
    else
    {
        getLogLog ().error (LOG4CPLUS_TEXT ("tostring_internal: conversion error"));
        result.clear ();
    }
}


std::string 
tostring (const std::wstring & src)
{
    std::string ret;
    tostring_internal (ret, src.c_str (), src.size ());
    return ret;
}


std::string 
tostring (wchar_t const * src)
{
    assert (src);
    std::string ret;
    tostring_internal (ret, src, std::wcslen (src));
    return ret;
}


static
void
towstring_internal (std::wstring & result, char const * src, std::size_t size)
{
    std::size_t result_size = size + 1;
    std::size_t ret;

    result.resize (result_size);
    while ((ret = mbstowcs (&result[0], src, result_size)) == result_size)
    {
        result_size *= 2;
        result.resize (result_size);
    }

    if (ret != static_cast<std::size_t>(-1))
        result.resize (ret);
    else
    {
        getLogLog ().error (LOG4CPLUS_TEXT ("towstring_internal: conversion error"));
        result.clear ();
    }
}


std::wstring 
towstring (const std::string& src)
{
    std::wstring ret;
    towstring_internal (ret, src.c_str (), src.size ());
    return ret;
}


std::wstring 
towstring (char const * src)
{
    assert (src);
    std::wstring ret;
    towstring_internal (ret, src, std::strlen (src));
    return ret;
}

#endif // LOG4CPLUS_WORKING_C_LOCALE

#endif // UNICODE

} // namespace helpers

} // namespace log4cplus
