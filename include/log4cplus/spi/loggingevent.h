// Module:  Log4CPLUS
// File:    loggingevent.h
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

#ifndef _LOG4CPLUS_SPI_INTERNAL_LOGGING_EVENT_HEADER_
#define _LOG4CPLUS_SPI_INTERNAL_LOGGING_EVENT_HEADER_

#include <log4cplus/config.h>
#include <log4cplus/loglevel.h>
#include <log4cplus/ndc.h>
#include <log4cplus/tstring.h>
#include <log4cplus/helpers/timehelper.h>
#include <log4cplus/helpers/threads.h>

namespace log4cplus {
    namespace spi {
        /**
         * The internal representation of logging events. When an affirmative
         * decision is made to log then a <code>InternalLoggingEvent</code> 
         * instance is created. This instance is passed around to the 
         * different log4cplus components.
         *
         * <p>This class is of concern to those wishing to extend log4cplus. 
         */
        struct LOG4CPLUS_EXPORT InternalLoggingEvent {
          // Ctors
             /**
              * Instantiate a LoggingEvent from the supplied parameters.
              * <p>
              * @param logger The logger of this event.
              * @param ll       The LogLevel of this event.
              * @param message  The message of this event.
              */
             InternalLoggingEvent(const log4cplus::tstring& logger,
                                  LogLevel ll,
                                  const log4cplus::tstring& message,
                                  const char* filename,
                                  int line)
              : loggerName(logger),
                ll(ll),
                ndc(log4cplus::getNDC().get()),
                message(message),
                thread( LOG4CPLUS_GET_CURRENT_THREAD_NAME ),
                timestamp(log4cplus::helpers::Time::gettimeofday()),
                file( filename ? LOG4CPLUS_C_STR_TO_TSTRING(filename) 
                               : log4cplus::tstring() ),
                line(line)
             {
             }

             InternalLoggingEvent(const log4cplus::tstring& logger,
                                  LogLevel ll,
                                  const log4cplus::tstring& ndc,
                                  const log4cplus::tstring& message,
                                  const log4cplus::tstring& thread,
                                  log4cplus::helpers::Time time,
                                  const log4cplus::tstring& file,
                                  int line)
              : loggerName(logger),
                ll(ll),
                ndc(ndc),
                message(message),
                thread(thread),
                timestamp(time),
                file(file),
                line(line)
             {
             }


          // Data
            /** The logger of the logging event. It is set by 
             * the LoggingEvent constructor. */
            const log4cplus::tstring loggerName;

            /** LogLevel of logging event. */
            const LogLevel ll;

            /** The nested diagnostic context (NDC) of logging event. */
            const log4cplus::tstring ndc;

            /** The application supplied message of logging event. */
            const log4cplus::tstring message;

            /** The name of thread in which this logging event was generated. */
            const log4cplus::tstring thread;

            /** The number of milliseconds elapsed from 1/1/1970 until logging event
             *  was created. */
            const log4cplus::helpers::Time timestamp;

            /** The is the file where this log statement was written */
            const log4cplus::tstring file;

            /** The is the line where this log statement was written */
            const int line;
        };

    } // end namespace spi
} // end namespace log4cplus

#endif // _LOG4CPLUS_SPI_INTERNAL_LOGGING_EVENT_HEADER_

