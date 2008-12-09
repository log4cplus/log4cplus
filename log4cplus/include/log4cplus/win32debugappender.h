// Module:  Log4CPLUS
// File:    win32debugappender.h
// Created: 12/2003
// Author:  Eduardo Francos, Odalio SARL
//
//
// Copyright (C) Odalio SARL. All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

/** @file */

#ifndef _LOG4CPLUS_WIN32DEBUG_APPENDER_HEADER_
#define _LOG4CPLUS_WIN32DEBUG_APPENDER_HEADER_

#include <log4cplus/config.hxx>
#if defined(_WIN32)

#include <log4cplus/appender.h>
#include <log4cplus/helpers/property.h>


namespace log4cplus {

    /**
     * Appends log events to a file. 
     */
    class LOG4CPLUS_EXPORT Win32DebugAppender
        : public Appender
    {
    public:
      // Ctors
        Win32DebugAppender();
        Win32DebugAppender(const log4cplus::helpers::Properties& properties);

      // Dtor
        virtual ~Win32DebugAppender();

      // Methods
        virtual void close();

    protected:
        virtual void append(const log4cplus::spi::InternalLoggingEvent& event);

    private:
      // Disallow copying of instances of this class
        Win32DebugAppender(const Win32DebugAppender&);
        Win32DebugAppender& operator=(const Win32DebugAppender&);
    };

} // end namespace log4cplus

#endif // _WIN32
#endif // _LOG4CPLUS_WIN32DEBUG_APPENDER_HEADER_

