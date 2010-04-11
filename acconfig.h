/* Module:  Log4CPLUS
 * File:    config.h.in
 * Created: 6/2001
 * Author:  Tad E. Smith
 *
 *
 * Copyright (C) The Apache Software Foundation. All rights reserved.
 *
 * This software is published under the terms of the Apache Software
 * License version 1.1, a copy of which has been included with this
 * distribution in the LICENSE.APL file.
 */

@TOP@

/* Define if your <sys/socket.h> declares type socklen_t.  */
#undef socklen_t

@BOTTOM@

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

#include <log4cplus/helpers/thread-config.h>

