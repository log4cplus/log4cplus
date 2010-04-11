// Module:  Log4CPLUS
// File:    nullappender.h
// Created: 6/2003
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

#ifndef _LOG4CPLUS_NULL_APPENDER_HEADER_
#define _LOG4CPLUS_NULL_APPENDER_HEADER_

#include <log4cplus/config.hxx>
#include <log4cplus/appender.h>
#include <log4cplus/helpers/property.h>


namespace log4cplus {

    /**
     * Appends log events to a file. 
     */
    class LOG4CPLUS_EXPORT NullAppender : public Appender {
    public:
      // Ctors
        NullAppender();
        NullAppender(const log4cplus::helpers::Properties& properties);

      // Dtor
        virtual ~NullAppender();

      // Methods
        virtual void close();

    protected:
        virtual void append(const log4cplus::spi::InternalLoggingEvent& event);

    private:
      // Disallow copying of instances of this class
        NullAppender(const NullAppender&);
        NullAppender& operator=(const NullAppender&);
    };

} // end namespace log4cplus

#endif // _LOG4CPLUS_NULL_APPENDER_HEADER_

