// Module:  Log4CPLUS
// File:    strftime.h
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

#ifndef _LOG4CPLUS_HELPERS_STRFTIME_HEADER_
#define _LOG4CPLUS_HELPERS_STRFTIME_HEADER_

#include <log4cplus/config.h>
#include <log4cplus/tstring.h>

#include <time.h>

namespace log4cplus {
    namespace helpers {
        LOG4CPLUS_EXPORT size_t strftime(tchar* s, size_t max, 
			                 const tchar* format, const struct tm* tm);
    }
}


#endif // _LOG4CPLUS_HELPERS_STRFTIME_HEADER_

