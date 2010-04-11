// Module:  Log4CPLUS
// File:    sleep.h
// Created: 5/2003
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

#ifndef _LOG4CPLUS_HELPERS_SLEEP_HEADER_
#define _LOG4CPLUS_HELPERS_SLEEP_HEADER_

#include <log4cplus/config.h>


namespace log4cplus {
    namespace helpers {
        LOG4CPLUS_EXPORT void sleep(unsigned long secs, 
                                    unsigned long nanosecs = 0);
        LOG4CPLUS_EXPORT void sleepmillis(unsigned long millis);
    } // end namespace helpers
} // end namespace log4cplus

#endif // _LOG4CPLUS_HELPERS_SLEEP_HEADER_

