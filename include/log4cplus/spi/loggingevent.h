// Module:  Log4CPLUS
// File:    loggingevent.h
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

#ifndef _LOG4CPLUS_SPI_INTERNAL_LOGGING_EVENT_HEADER_
#define _LOG4CPLUS_SPI_INTERNAL_LOGGING_EVENT_HEADER_

#include <log4cplus/config.hxx>
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
         * This class is of concern to those wishing to extend log4cplus. 
         */
        class LOG4CPLUS_EXPORT InternalLoggingEvent {
        public:
          // Ctors
             /**
              * Instantiate a LoggingEvent from the supplied parameters.
              * 
              * @param logger   The logger of this event.
              * @param ll_      The LogLevel of this event.
              * @param message_ The message of this event.
              * @param filename Name of file where this event has occurred,
              * can be NULL.
              * @param line_    Line number in file specified by
              *                 the <code>filename</code> parameter.
              */
             InternalLoggingEvent(const log4cplus::tstring& logger,
                                  LogLevel ll_,
                                  const log4cplus::tstring& message_,
                                  const char* filename,
                                  int line_)
              : message(message_),
                loggerName(logger),
                ll(ll_),
                ndc(),
                thread(),
                timestamp(log4cplus::helpers::Time::gettimeofday()),
                file( (  filename
                       ? LOG4CPLUS_C_STR_TO_TSTRING(filename) 
                       : log4cplus::tstring()) ),
                line(line_),
                threadCached(false),
                ndcCached(false)
             {
             }

             InternalLoggingEvent(const log4cplus::tstring& logger,
                                  LogLevel ll_,
                                  const log4cplus::tstring& ndc_,
                                  const log4cplus::tstring& message_,
                                  const log4cplus::tstring& thread_,
                                  log4cplus::helpers::Time time,
                                  const log4cplus::tstring& file_,
                                  int line_)
              : message(message_),
                loggerName(logger),
                ll(ll_),
                ndc(ndc_),
                thread(thread_),
                timestamp(time),
                file(file_),
                line(line_),
                threadCached(true),
                ndcCached(true)
             {
             }

             InternalLoggingEvent(const log4cplus::spi::InternalLoggingEvent& rhs)
              : message(rhs.getMessage()),
                loggerName(rhs.getLoggerName()),
                ll(rhs.getLogLevel()),
                ndc(rhs.getNDC()),
                thread(rhs.getThread()),
                timestamp(rhs.getTimestamp()),
                file(rhs.getFile()),
                line(rhs.getLine()),
                threadCached(true),
                ndcCached(true)
             {
             }

            virtual ~InternalLoggingEvent();


          // public virtual methods
            /** The application supplied message of logging event. */
            virtual const log4cplus::tstring& getMessage() const;

            /** Returns the 'type' of InternalLoggingEvent.  Derived classes
             *  should override this method.  (NOTE: Values <= 1000 are
             *  reserved for log4cplus and should not be used.)
             */
            virtual unsigned int getType() const;

           /** Returns a copy of this object.  Derived classes
             *  should override this method.
	     */
            virtual std::auto_ptr<InternalLoggingEvent> clone() const;



          // public methods
            /** The logger of the logging event. It is set by 
             *  the LoggingEvent constructor. 
	     */
            const log4cplus::tstring& getLoggerName() const { return loggerName; }

            /** LogLevel of logging event. */
            LogLevel getLogLevel() const { return ll; }

            /** The nested diagnostic context (NDC) of logging event. */
            const log4cplus::tstring& getNDC() const { 
                if(!ndcCached) {
                    ndc = log4cplus::getNDC().get();
                    ndcCached = true;
                }
                return ndc; 
            }

            /** The name of thread in which this logging event was generated. */
            const log4cplus::tstring& getThread() const {
                if(!threadCached) {
                    thread = LOG4CPLUS_GET_CURRENT_THREAD_NAME;
                    threadCached = true;
                }
                return thread; 
            }

            /** The number of milliseconds elapsed from 1/1/1970 until logging event
             *  was created. */
            const log4cplus::helpers::Time& getTimestamp() const { return timestamp; }

            /** The is the file where this log statement was written */
            const log4cplus::tstring& getFile() const { return file; }

            /** The is the line where this log statement was written */
            int getLine() const { return line; }
 
          // public operators
            log4cplus::spi::InternalLoggingEvent&
            operator=(const log4cplus::spi::InternalLoggingEvent& rhs);

          // static methods
            static unsigned int getDefaultType();

        protected:
          // Data
            log4cplus::tstring message;

        private:
            log4cplus::tstring loggerName;
            LogLevel ll;
            mutable log4cplus::tstring ndc;
            mutable log4cplus::tstring thread;
            log4cplus::helpers::Time timestamp;
            log4cplus::tstring file;
            int line;
            /** Indicates whether or not the Threadname has been retrieved. */
            mutable bool threadCached;
            /** Indicates whether or not the NDC has been retrieved. */
            mutable bool ndcCached;
        };

    } // end namespace spi
} // end namespace log4cplus

#endif // _LOG4CPLUS_SPI_INTERNAL_LOGGING_EVENT_HEADER_

