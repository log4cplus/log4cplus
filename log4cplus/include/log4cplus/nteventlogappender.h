// Module:  Log4CPLUS
// File:    nteventlogappender.h
// Created: 4/2003
// Author:  Michael CATANZARITI
//
// Copyright (C) Michael CATANZARITI  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

/** @file */

#ifndef _LOG4CPLUS_NT_EVENT_LOG_APPENDER_HEADER_
#define _LOG4CPLUS_NT_EVENT_LOG_APPENDER_HEADER_

#include <log4cplus/config.h>
#if defined(_WIN32)

#include <log4cplus/appender.h>


namespace log4cplus {

    /**
     * Appends log events to NT EventLog. 
     */
    class LOG4CPLUS_EXPORT NTEventLogAppender : public Appender {
    public:
      // ctors
        NTEventLogAppender(const log4cplus::tstring& server, 
                           const log4cplus::tstring& log, 
                           const log4cplus::tstring& source);
        NTEventLogAppender(const log4cplus::helpers::Properties properties);

      // dtor
        virtual ~NTEventLogAppender();

      // public Methods
        virtual void close();

    protected:
        virtual void append(const spi::InternalLoggingEvent& event);
        virtual WORD getEventType(const spi::InternalLoggingEvent& event);
        virtual WORD getEventCategory(const spi::InternalLoggingEvent& event);
        void init();

        /*
         * Add this source with appropriate configuration keys to the registry.
         */
        void addRegistryInfo();

      // Data
        log4cplus::tstring server;
        log4cplus::tstring log;
        log4cplus::tstring source;
        HANDLE hEventLog;
        SID* pCurrentUserSID;

    private:
      // Disallow copying of instances of this class
        NTEventLogAppender(const NTEventLogAppender&);
        NTEventLogAppender& operator=(const NTEventLogAppender&);
    };

} // end namespace log4cplus

#endif // _WIN32
#endif //_LOG4CPLUS_NT_EVENT_LOG_APPENDER_HEADER_

