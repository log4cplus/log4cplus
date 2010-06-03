// Module:  Log4CPLUS
// File:    syslogappender.h
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright 2001-2009 Tad E. Smith
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

#ifndef _LOG4CPLUS_SYSLOG_APPENDER_HEADER_
#define _LOG4CPLUS_SYSLOG_APPENDER_HEADER_

#include <log4cplus/config.hxx>

#if defined(LOG4CPLUS_HAVE_SYSLOG_H) && !defined(_WIN32)
#include <log4cplus/appender.h>

namespace log4cplus {

    /**
     * Appends log events to a file. 
     *
     * <h3>Properties</h3>
     * <dl>
     * <dt><tt>ident</tt></dt>
     * <dd>First argument to <code>openlog()</code>, a string that
     * will be prepended to every message.</dd>
     * 
     * <dt><tt>facility</tt></dt>
     * <dd>Facility is used in combination with syslog level in first
     * argument to syslog(). It can be one of the supported facility
     * names (case insensitive), e.g. auth, cron, kern, mail, news
     * etc.</dd>
     * </dl>
     */
    class LOG4CPLUS_EXPORT SysLogAppender : public Appender {
    public:
      // Ctors
        SysLogAppender(const tstring& ident);
        SysLogAppender(const log4cplus::helpers::Properties & properties);

      // Dtor
        virtual ~SysLogAppender();

      // Methods
        virtual void close();

    protected:
        virtual int getSysLogLevel(const LogLevel& ll) const;
        virtual void append(const spi::InternalLoggingEvent& event);

      // Data
        tstring ident;
        int facility;

    private:
      // Disallow copying of instances of this class
        SysLogAppender(const SysLogAppender&);
        SysLogAppender& operator=(const SysLogAppender&);

        std::string identStr;
    };

} // end namespace log4cplus

#endif // defined(HAVE_SYSLOG_H)

#endif // _LOG4CPLUS_SYSLOG_APPENDER_HEADER_

