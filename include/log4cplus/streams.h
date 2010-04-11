// Module:  Log4CPLUS
// File:    streams.h
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

