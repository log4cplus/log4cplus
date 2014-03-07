//  Copyright (C) 2010-2013, Vaclav Haisman. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without modifica-
//  tion, are permitted provided that the following conditions are met:
//
//  1. Redistributions of  source code must  retain the above copyright  notice,
//     this list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED WARRANTIES,
//  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS  FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN NO  EVENT SHALL  THE
//  APACHE SOFTWARE  FOUNDATION  OR ITS CONTRIBUTORS  BE LIABLE FOR  ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLU-
//  DING, BUT NOT LIMITED TO, PROCUREMENT  OF SUBSTITUTE GOODS OR SERVICES; LOSS
//  OF USE, DATA, OR  PROFITS; OR BUSINESS  INTERRUPTION)  HOWEVER CAUSED AND ON
//  ANY  THEORY OF LIABILITY,  WHETHER  IN CONTRACT,  STRICT LIABILITY,  OR TORT
//  (INCLUDING  NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT OF THE  USE OF
//  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <log4cplus/helpers/snprintf.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/internal/internal.h>
#include <cstdarg>
#include <cstdio>
#if defined (UNICODE)
#include <cwchar>
#endif


namespace log4cplus { namespace helpers {


std::size_t const START_BUF_SIZE = 512;

namespace
{


static inline int vsntprintf (tchar * dest, std::size_t dest_size,
    tchar const * fmt, std::va_list args)
    LOG4CPLUS_FORMAT_ATTRIBUTE (__printf__, 3, 0);

static inline
int
vsntprintf (tchar * dest, std::size_t dest_size, tchar const * fmt,
    std::va_list args)
{
    int ret;

#if defined (UNICODE)
    ret = std::vswprintf (dest, dest_size, fmt, args);
#else
    ret = std::vsnprintf (dest, dest_size, fmt, args);
#endif

    return ret;
}


} // namespace {


snprintf_buf::snprintf_buf ()
    : buf (START_BUF_SIZE)
{ }


tchar const *
snprintf_buf::print (tchar const * fmt, ...)
{
    assert (fmt);

    tchar const * str = 0;
    int ret = 0;
    std::va_list args;

    do
    {
        va_start (args, fmt);
        ret = print_va_list (str, fmt, args);
        va_end (args);
    }
    while (ret == -1);

    return str;
}


int
snprintf_buf::print_va_list (tchar const * & str, tchar const * fmt,
    std::va_list args)
{
    int printed;
    std::size_t const fmt_len = std::char_traits<tchar>::length (fmt);
    std::size_t buf_size = buf.size ();
    std::size_t const output_estimate = fmt_len + fmt_len / 2 + 1;
    if (output_estimate > buf_size)
        buf.resize (buf_size = output_estimate);

    printed = vsntprintf (&buf[0], buf_size - 1, fmt, args);
    if (printed >= static_cast<int>(buf_size - 1))
    {
        buf_size = printed + 2;
        buf.resize (buf_size);
        printed = -1;
    }
    else if (printed == -1)
    {
        buf[0] = 0;
        printed = 0;
    }

    str = &buf[0];
    return printed;
}


} } // namespace log4cplus { namespace helpers
