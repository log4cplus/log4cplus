// Module:  Log4CPLUS
// File:    consoleappender.h
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

#ifndef _LOG4CPLUS_CONSOLE_APPENDER_HEADER_
#define _LOG4CPLUS_CONSOLE_APPENDER_HEADER_

#include <log4cplus/config.h>
#include <log4cplus/appender.h>

namespace log4cplus {
    /**
     * ConsoleAppender appends log events to <code>System.out</code> or
     * <code>System.err</code> using a layout specified by the
     * user. The default target is <code>System.out</code>.
     */
    class LOG4CPLUS_EXPORT ConsoleAppender : public Appender {
    public:
      // Ctors
        ConsoleAppender(bool logToStdErr = false, bool immediateFlush = false);
        ConsoleAppender(const log4cplus::helpers::Properties properties);

      // Dtor
        ~ConsoleAppender();

      // Methods
        virtual void close();

    protected:
        virtual void append(const spi::InternalLoggingEvent& event);

      // Data
        bool logToStdErr;
        /**
         * Immediate flush means that the underlying output stream
         * will be flushed at the end of each append operation.
         */
        bool immediateFlush;
    };

} // end namespace log4cplus

#endif // _LOG4CPLUS_CONSOLE_APPENDER_HEADER_

