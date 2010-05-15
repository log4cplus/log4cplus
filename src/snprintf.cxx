#include <log4cplus/helpers/snprintf.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/internal/internal.h>
#include <cstdarg>
#include <cstdio>
#if defined (UNICODE)
#include <cwchar>
#endif
#if defined (LOG4CPLUS_HAVE_STDARG_H)
#  include <stdarg.h>
#endif
#if defined (LOG4CPLUS_HAVE_WCHAR_H)
#  include <wchar.h>
#endif
#if defined (LOG4CPLUS_HAVE_STDIO_H)
#  include <stdio.h>
#endif


namespace log4cplus { namespace helpers {


size_t const START_BUF_SIZE = 512;

#if defined (WIN32)
char const NULL_FILE[] = "NUL";
#else
char const NULL_FILE[] = "/dev/null";
#endif


namespace
{


static inline
int
vftprintf (std::FILE * file, tchar const * fmt, std::va_list args)
{
#if defined (UNICODE)
    return std::vfwprintf (file, fmt, args);
#else
    return std::vfprintf (file, fmt, args);
#endif
}


static inline
int
vstprintf (tchar * dest, std::size_t dest_size, tchar const * fmt,
    std::va_list args)
{
    int ret;
#if defined (UNICODE)
    ret = std::vswprintf (dest, dest_size, fmt, args);
#else
    ret = std::vsprintf (dest, fmt, args);
#endif

    if (ret >= 0)
        assert (ret <= dest_size);
    
    return ret;
}


}


snprintf_buf::snprintf_buf ()
    : buf (START_BUF_SIZE)
{ }


tchar const *
snprintf_buf::print (tchar const * fmt, ...)
{
    assert (fmt);

    std::va_list args;
    va_start (args, fmt);
    tchar const * ret = print (fmt, args);
    va_end (args);
    return ret;
}


tchar const *
snprintf_buf::print (tchar const * fmt, std::va_list args)
{
    int printed;
    size_t const fmt_len = char_traits::length (fmt);
    size_t buf_size = buf.size ();
    size_t const output_estimate = fmt_len + fmt_len / 2 + 1;
    if (output_estimate > buf_size)
        buf.resize (output_estimate);

#if defined (LOG4CPLUS_USE_POOR_MANS_SNPRINTF)
    std::FILE * & fnull = internal::get_ptd ()->fnull;
    if (! fnull)
    {
        fnull = std::fopen (NULL_FILE, "wb");
        if (! fnull)
        {
            LogLog::getLogLog ()->error (
                LOG4CPLUS_TEXT ("Could not open NULL_FILE."));
            buf.clear ();
            buf.push_back (0);
            return &buf[0];
        }
    }
   
    printed = vftprintf (fnull, fmt, args);
    if (printed == -1)
    {
        LogLog::getLogLog ()->error (
            LOG4CPLUS_TEXT ("Error printing into NULL_FILE."));
        buf.clear ();
        buf.push_back (0);
        return &buf[0];
    }

    buf.resize (printed + 1);
    int sprinted = vstprintf (&buf[0], buf.size (), fmt, args);
    if (sprinted == -1)
    {
        LogLog::getLogLog ()->error (
            LOG4CPLUS_TEXT ("Error printing into string."));
        buf.clear ();
        buf.push_back (0);
        return &buf[0];
    }
    assert (printed == sprinted);
    
    buf[sprinted] = 0;

#else
    do
    {
#  if defined (UNICODE)
        printed = std::vswprintf (&buf[0], buf_size - 1, fmt, args);

#  elif defined (LOG4CPLUS_HAVE_VSNPRINTF)
        printed = vsnprintf (&buf[0], buf_size - 1, fmt, args);

#  elif defined (LOG4CPLUS_HAVE__VSNPRINTF)
        printed = _vsnprintf (&buf[0], buf_size - 1, fmt, args);

#  else
#    error "Neither vsnprintf nor _vsnprintf is available."

#  endif

        if (printed == -1)
        {
            buf_size *= 2;
            buf.resize (buf_size);
        }
        else
            buf[printed] = 0;
    }
    while (printed == -1);

#endif

    return &buf[0];
}


} } // namespace log4cplus { namespace helpers
