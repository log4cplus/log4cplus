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

#include <log4cplus/tstring.h>


namespace log4cplus { namespace internal {


extern log4cplus::tstring const empty_str;


} } // namespace log4cplus { namespace internal {

#endif // LOG4CPLUS_INTERNAL_INTERNAL_HEADER_
