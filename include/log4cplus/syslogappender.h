// Module:  Log4CPLUS
// File:    syslogappender.h
// Created: 6/2001
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

#ifndef _LOG4CPLUS_SYSLOG_APPENDER_HEADER_
#define _LOG4CPLUS_SYSLOG_APPENDER_HEADER_

#include <log4cplus/config.hxx>

#if defined(LOG4CPLUS_HAVE_SYSLOG_H) && !defined(_WIN32)
#include <log4cplus/appender.h>

namespace log4cplus {

    /**
     * Appends log events to a file. 
     */
    class LOG4CPLUS_EXPORT SysLogAppender : public Appender {
    public:
      // Ctors
        SysLogAppender(const tstring& ident);
        SysLogAppender(const log4cplus::helpers::Properties properties);

      // Dtor
        virtual ~SysLogAppender();

      // Methods
        virtual void close();

    protected:
        virtual int getSysLogLevel(const LogLevel& ll) const;
        virtual void append(const spi::InternalLoggingEvent& event);

      // Data
        tstring ident;

    private:
      // Disallow copying of instances of this class
        SysLogAppender(const SysLogAppender&);
        SysLogAppender& operator=(const SysLogAppender&);
    };

} // end namespace log4cplus

#endif // defined(HAVE_SYSLOG_H)

#endif // _LOG4CPLUS_SYSLOG_APPENDER_HEADER_

