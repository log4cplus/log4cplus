// Module:  Log4CPLUS
// File:    sleep.h
// Created: 5/2003
// Author:  Tad E. Smith
//
//
// Copyright 2003-2009 Tad E. Smith
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

#ifndef _LOG4CPLUS_HELPERS_SLEEP_HEADER_
#define _LOG4CPLUS_HELPERS_SLEEP_HEADER_

#include <log4cplus/config.hxx>


namespace log4cplus {
    namespace helpers {
        LOG4CPLUS_EXPORT void sleep(unsigned long secs, 
                                    unsigned long nanosecs = 0);
        LOG4CPLUS_EXPORT void sleepmillis(unsigned long millis);
    } // end namespace helpers
} // end namespace log4cplus

#endif // _LOG4CPLUS_HELPERS_SLEEP_HEADER_

