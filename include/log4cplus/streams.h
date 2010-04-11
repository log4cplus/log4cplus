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
#  define LOG4CPLUS_STREAM_NAMESPACE std
#elif defined(HAVE_STRSTREAM)
#  include <strstream>
#  if defined(__DECCXX) && !defined(__USE_STD_IOSTREAM)
#    define LOG4CPLUS_STREAM_NAMESPACE
#  else
#    define LOG4CPLUS_STREAM_NAMESPACE std
#  endif
#elif defined(HAVE_STRSTREAM_H)
#  include <strstream.h>
#  define LOG4CPLUS_STREAM_NAMESPACE
#else
#  error "There doesn't appear to be any s*stream headers!!"
#endif

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
#ifdef HAVE_SSTREAM
        typedef LOG4CPLUS_STREAM_NAMESPACE::ostringstream tostringstream;
#else
        class tostringstream : public LOG4CPLUS_STREAM_NAMESPACE::ostrstream {
        public:
            tstring str() { 
                char *ptr = LOG4CPLUS_STREAM_NAMESPACE::ostrstream::str(); 
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

