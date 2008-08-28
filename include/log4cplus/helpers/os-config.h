#ifndef LOG4CPLUS_INCLUDE_LOG4CPLUS_OS_CONFIG_H
#define LOG4CPLUS_INCLUDE_LOG4CPLUS_OS_CONFIG_H

#include <log4cplus/config.h>


#ifdef _WIN32
#  include <log4cplus/config-win32.h>

#elif (defined(__APPLE__) || (defined(__MWERKS__) && defined(__MACOS__)))
#  include <log4cplus/config-macosx.h>

#endif // _WIN32

#if !defined(_WIN32)
#  if !defined(LOG4CPLUS_SINGLE_THREADED)
#    define LOG4CPLUS_USE_PTHREADS
#  endif
#  define LOG4CPLUS_EXPORT

#endif // !_WIN32


#endif // !LOG4CPLUS_INCLUDE_LOG4CPLUS_OS_CONFIG_H
