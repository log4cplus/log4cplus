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

#include <log4cplus/config.h>
#include <log4cplus/tstring.h>

#include <iostream>
#ifdef HAVE_SSTREAM
#  include <sstream>
#elif defined(HAVE_STRSTREAM)
#  include <strstream>
#else
#  error "There doesn't appear to be any s*stream headers!!"
#endif

#ifdef UNICODE
    namespace log4cplus {
        typedef std::wostream tostream;
        typedef std::wistream tistream;
        typedef std::wostringstream tostringstream;
        static tostream &tcout = std::wcout;
        static tostream &tcerr = std::wcerr;
    }
#else
    namespace log4cplus {
        typedef std::ostream tostream;
        typedef std::istream tistream;
        static tostream &tcout = std::cout;
        static tostream &tcerr = std::cerr;
#ifdef HAVE_SSTREAM
        typedef std::ostringstream tostringstream;
#else
        class tostringstream : public std::ostrstream {
        public:
            tstring str() { 
                char *ptr = std::ostrstream::str(); 
                if(ptr) {
                    return tstring(ptr, pcount());
                }
                else {
                    return tstring();
                }
            }
        };

#endif // HAVE_SSTREAM
    }
#endif // UNICODE

#endif // LOG4CPLUS_STREAMS_HEADER_

