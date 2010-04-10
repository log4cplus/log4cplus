// Module:  Log4CPLUS
// File:    streams.h
// Created: 4/2003
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

#ifndef LOG4CPLUS_STREAMS_HEADER_
#define LOG4CPLUS_STREAMS_HEADER_

#include <log4cplus/config.hxx>
#include <log4cplus/tstring.h>

#include <iostream>
#include <sstream>
#define LOG4CPLUS_STREAM_NAMESPACE std

#ifdef UNICODE
    namespace log4cplus {
        typedef LOG4CPLUS_STREAM_NAMESPACE::wostream tostream;
        typedef LOG4CPLUS_STREAM_NAMESPACE::wistream tistream;
        typedef LOG4CPLUS_STREAM_NAMESPACE::wostringstream tostringstream;
        static tostream &tcout = LOG4CPLUS_STREAM_NAMESPACE::wcout;
        static tostream &tcerr = LOG4CPLUS_STREAM_NAMESPACE::wcerr;
    }

LOG4CPLUS_EXPORT log4cplus::tostream& operator <<(log4cplus::tostream&, const char* psz );

#else
    namespace log4cplus {
        typedef LOG4CPLUS_STREAM_NAMESPACE::ostream tostream;
        typedef LOG4CPLUS_STREAM_NAMESPACE::istream tistream;
        static tostream &tcout = LOG4CPLUS_STREAM_NAMESPACE::cout;
        static tostream &tcerr = LOG4CPLUS_STREAM_NAMESPACE::cerr;
        typedef LOG4CPLUS_STREAM_NAMESPACE::ostringstream tostringstream;
    }
#endif // UNICODE

#endif // LOG4CPLUS_STREAMS_HEADER_

