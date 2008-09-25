#ifndef LOG4CPLUS_CONFIG_HXX
#define LOG4CPLUS_CONFIG_HXX


#if defined (_WIN32)
#  include <log4cplus/config/win32.h>
#elif (defined(__MWERKS__) && defined(__MACOS__))
#  include <log4cplus/config/macosx.h>
#else
#  include <log4cplus/config/defines.hxx>
#endif

#if !defined(_WIN32)
#  if !defined(LOG4CPLUS_SINGLE_THREADED)
#    define LOG4CPLUS_USE_PTHREADS
#  endif
#  if defined (INSIDE_LOG4CPLUS)
#    define LOG4CPLUS_EXPORT LOG4CPLUS_DECLSPEC_EXPORT
#  else
#    define LOG4CPLUS_EXPORT LOG4CPLUS_DECLSPEC_IMPORT
#  endif // defined (INSIDE_LOG4CPLUS)
#endif // !_WIN32

#include <log4cplus/helpers/thread-config.h>


#endif // LOG4CPLUS_CONFIG_HXX
