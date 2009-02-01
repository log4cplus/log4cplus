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


#ifdef UNICODE
namespace log4cplus
{
    typedef std::wostream tostream;
    typedef std::wistream tistream;
    typedef std::wostringstream tostringstream;
    extern LOG4CPLUS_EXPORT tostream & tcout;
    extern LOG4CPLUS_EXPORT tostream & tcerr;
}

LOG4CPLUS_EXPORT log4cplus::tostream& operator <<(log4cplus::tostream&, const char* psz );

#else
namespace log4cplus
{
    typedef std::ostream tostream;
    typedef std::istream tistream;
    typedef std::ostringstream tostringstream;
    extern LOG4CPLUS_EXPORT tostream & tcout;
    extern LOG4CPLUS_EXPORT tostream & tcerr;
}

#endif // UNICODE

#endif // LOG4CPLUS_STREAMS_HEADER_

