// Module:  Log4CPLUS
// File:    internal.h
// Created: 1/2009
// Author:  Vaclav Haisman
//
//
// Copyright (C) Vaclav Haisman  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

/** @file 
 * This header contains declaration internal to log4cplus. They must never be
 * visible from user accesible headers or exported in DLL/shared libray.
 */


#ifndef LOG4CPLUS_INTERNAL_INTERNAL_HEADER_
#define LOG4CPLUS_INTERNAL_INTERNAL_HEADER_

#if ! defined (INSIDE_LOG4CPLUS)
#  error "This header must not be be used outside log4cplus' implementation files."
#endif

#include <log4cplus/config.hxx>
#include <log4cplus/tstring.h>
#include <log4cplus/streams.h>
#include <log4cplus/ndc.h>


namespace log4cplus { namespace internal {


//! Canonical empty string. It is used when the need to return empty string
//! by reference arises.
extern log4cplus::tstring const empty_str;


#if ! defined (LOG4CPLUS_SINGLE_THREADED)

//! Per thread data.
struct per_thread_data
{
    per_thread_data ()
    { }

    tostringstream macros_oss;
    DiagnosticContextStack ndc_dcs;
};


#if defined (LOG4CPLUS_THREAD_LOCAL_VAR)

extern LOG4CPLUS_THREAD_LOCAL_VAR per_thread_data * ptd;


inline
internal::per_thread_data *
get_ptd ()
{
    return ptd;
}


inline
void
set_ptd (per_thread_data * p)
{
    ptd = p;
}


#else

// TLS key whose value is pointer struct per_thread_data.
extern LOG4CPLUS_THREAD_LOCAL_TYPE tls_storage_key;


inline
void
set_ptd (per_thread_data * p)
{
    LOG4CPLUS_SET_THREAD_LOCAL_VALUE (internal::tls_storage_key, p);
}


inline
internal::per_thread_data *
get_ptd ()
{
    internal::per_thread_data * ptd
        = reinterpret_cast<internal::per_thread_data *>(
            LOG4CPLUS_GET_THREAD_LOCAL_VALUE (internal::tls_storage_key));

    if (! ptd)
    {
        ptd = new internal::per_thread_data;
        set_ptd (ptd);
    }

    return ptd;
}


#endif

#endif


} } // namespace log4cplus { namespace internal {

#endif // LOG4CPLUS_INTERNAL_INTERNAL_HEADER_
