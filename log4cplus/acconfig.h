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

@BOTTOM@

#ifdef WIN32
#  include <log4cplus/config-win32.h>

#else 
#  if !defined(LOG4CPLUS_SINGLE_THREADED)
#    define LOG4CPLUS_USE_PTHREADS
#  endif
#  define LOG4CPLUS_EXPORT

#endif // WIN32

#include <log4cplus/helpers/thread-config.h>

