// -*- C++ -*-
// Module:  Log4CPLUS
// File:    threadsafetyanalysis.h
// Created: 12/2023
// Author:  Vaclav Haisman
//
//
//  Copyright (C) 2023, Vaclav Haisman. All rights reserved.
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

/**
 * @file Thread safety analysis annotations/attributes for Clang.
 *
 */

#ifndef LOG4CPLUS_INTERNAL_THREADSAFETYANALYSIS_HEADER_
#define LOG4CPLUS_INTERNAL_THREADSAFETYANALYSIS_HEADER_

#if defined (__clang) && \
    ! defined (SWIG)
#define LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(x)   __attribute__((x))
#else // ! defined (__clang)
#define LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(x)   // no-op
#endif // defined (__clang)

#define LOG4CPLUS_TSA_CAPABILITY(x) \
  LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(capability(x))

#define LOG4CPLUS_TSA_SCOPED_CAPABILITY \
  LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(scoped_lockable)

#define LOG4CPLUS_TSA_GUARDED_BY(x) \
  LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(guarded_by(x))

#define LOG4CPLUS_TSA_PT_GUARDED_BY(x) \
  LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(pt_guarded_by(x))

#define LOG4CPLUS_TSA_ACQUIRED_BEFORE(...) \
  LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(acquired_before(__VA_ARGS__))

#define LOG4CPLUS_TSA_ACQUIRED_AFTER(...) \
  LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(acquired_after(__VA_ARGS__))

#define LOG4CPLUS_TSA_REQUIRES(...) \
  LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(requires_capability(__VA_ARGS__))

#define LOG4CPLUS_TSA_REQUIRES_SHARED(...) \
  LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(requires_shared_capability(__VA_ARGS__))

#define LOG4CPLUS_TSA_ACQUIRE(...) \
  LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(acquire_capability(__VA_ARGS__))

#define LOG4CPLUS_TSA_ACQUIRE_SHARED(...) \
  LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(acquire_shared_capability(__VA_ARGS__))

#define LOG4CPLUS_TSA_RELEASE(...) \
  LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(release_capability(__VA_ARGS__))

#define LOG4CPLUS_TSA_RELEASE_SHARED(...) \
  LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(release_shared_capability(__VA_ARGS__))

#define LOG4CPLUS_TSA_RELEASE_GENERIC(...) \
  LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(release_generic_capability(__VA_ARGS__))

#define LOG4CPLUS_TSA_TRY_ACQUIRE(...) \
  LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(try_acquire_capability(__VA_ARGS__))

#define LOG4CPLUS_TSA_TRY_ACQUIRE_SHARED(...) \
  LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(try_acquire_shared_capability(__VA_ARGS__))

#define LOG4CPLUS_TSA_EXCLUDES(...) \
  LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(locks_excluded(__VA_ARGS__))

#define LOG4CPLUS_TSA_ASSERT_CAPABILITY(x) \
  LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(assert_capability(x))

#define LOG4CPLUS_TSA_ASSERT_SHARED_CAPABILITY(x) \
  LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(assert_shared_capability(x))

#define LOG4CPLUS_TSA_RETURN_CAPABILITY(x) \
  LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(lock_returned(x))

#define LOG4CPLUS_TSA_NO_THREAD_SAFETY_ANALYSIS \
  LOG4CPLUS_TSA_ANNOTATION_ATTRIBUTE(no_thread_safety_analysis)

#endif // LOG4CPLUS_INTERNAL_THREADSAFETYANALYSIS_HEADER_
