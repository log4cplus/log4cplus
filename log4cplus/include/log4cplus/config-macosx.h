// Module:  Log4CPLUS
// File:    config-macosx.h
// Created: 7/2003
// Author:  Christopher R. Bailey
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

/** @file */

#ifndef LOG4CPLUS_CONFIG_MACOSX_HEADER_
#define LOG4CPLUS_CONFIG_MACOSX_HEADER_

#if (defined(__APPLE__) || (defined(__MWERKS__) && defined(__MACOS__)))

#define LOG4CPLUS_HAVE_GETTIMEOFDAY 1
#define socklen_t int

#endif // MACOSX
#endif // LOG4CPLUS_CONFIG_MACOSX_HEADER_
