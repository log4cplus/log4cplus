// -*- C++ -*-
// Module:  Log4CPLUS
// File:    thread-config.h
// Created: 4/2003
// Author:  Tad E. Smith
//
//
// Copyright 2003-2010 Tad E. Smith
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
/** @file */

#ifndef LOG4CPLUS_HELPERS_THREAD_CONFIG_HEADER_
#define LOG4CPLUS_HELPERS_THREAD_CONFIG_HEADER_

#if defined (LOG4CPLUS_USE_PTHREADS)
// Nothing.

#elif defined(LOG4CPLUS_USE_WIN32_THREADS)
#   if (_WIN32_WINNT + 0 >= 0x0600)
#     define LOG4CPLUS_USE_SRW_LOCK
#   else
#     define LOG4CPLUS_POOR_MANS_SHAREDMUTEX
#   endif
#   if defined (_MSC_VER)
#     undef LOG4CPLUS_HAVE_TLS_SUPPORT
#     undef LOG4CPLUS_THREAD_LOCAL_VAR
// Comment out the following two lines if you do intend to use log4cplus.dll
// for loading using LoadLibrary(). The __declspec(thread) functionality is not
// compatible with such DLL use. For more information why is this necessary see
// <http://msdn.microsoft.com/en-us/library/2s9wt68x(vs.80).aspx>.
#     define LOG4CPLUS_HAVE_TLS_SUPPORT 1
#     define LOG4CPLUS_THREAD_LOCAL_VAR __declspec(thread)
#   endif

#elif defined(LOG4CPLUS_SINGLE_THREADED)
#   undef LOG4CPLUS_HAVE_TLS_SUPPORT
#   undef LOG4CPLUS_THREAD_LOCAL_VAR

#else
#   error "You Must define a Threading model"

#endif


#endif // LOG4CPLUS_HELPERS_THREAD_CONFIG_HEADER_
