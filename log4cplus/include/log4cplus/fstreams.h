// Module:  Log4CPLUS
// File:    fstreams.h
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

#ifndef LOG4CPLUS_FSTREAMS_HEADER_
#define LOG4CPLUS_FSTREAMS_HEADER_

#include <log4cplus/config.h>

#include <fstream>

#ifdef UNICODE
    namespace log4cplus {
        typedef std::wofstream tofstream;
        typedef std::wifstream tifstream;
    }
#else
    namespace log4cplus {
        typedef std::ofstream tofstream;
        typedef std::ifstream tifstream;
    }
#endif // UNICODE

#endif // LOG4CPLUS_FSTREAMS_HEADER_

