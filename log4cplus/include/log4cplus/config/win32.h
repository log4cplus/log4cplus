// Module:  Log4CPLUS
// File:    config-win32.h
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

/** @file */

#ifndef LOG4CPLUS_CONFIG_WIN32_HEADER_
#define LOG4CPLUS_CONFIG_WIN32_HEADER_

#ifdef _WIN32
#include <windows.h>

/* Define if you have the ftime function.  */
#define LOG4CPLUS_HAVE_FTIME 1

#ifdef LOG4CPLUS_STATIC
#  define LOG4CPLUS_EXPORT
#else
#  if defined (LOG4CPLUS_BUILD_DLL) || defined (log4cplus_EXPORTS)
#    define LOG4CPLUS_EXPORT __declspec(dllexport)
#  else
#    define LOG4CPLUS_EXPORT __declspec(dllimport)
#  endif
#endif

#ifndef LOG4CPLUS_SINGLE_THREADED
#  define LOG4CPLUS_USE_WIN32_THREADS
#endif

#if defined(_MSC_VER)
  // Warning about: identifier was truncated to '255' characters in the debug information
#  pragma warning( disable : 4786 )
  // Warning about: <type1> needs to have dll-interface to be used by clients of class <type2>
#  pragma warning( disable : 4251 )

#  if _MSC_VER >= 1400
#    define LOG4CPLUS_WORKING_LOCALE
#  endif

#endif


#endif // _WIN32
#endif // LOG4CPLUS_CONFIG_WIN32_HEADER_

