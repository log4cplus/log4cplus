// Module:  Log4CPLUS
// File:    appender.h
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright 2001-2010 Tad E. Smith
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

#ifndef _LOG4CPLUS_APPENDER_HEADER_
#define _LOG4CPLUS_APPENDER_HEADER_

#include <log4cplus/config.hxx>
#include <log4cplus/layout.h>
#include <log4cplus/loglevel.h>
#include <log4cplus/tstring.h>
#include <log4cplus/helpers/logloguser.h>
#include <log4cplus/helpers/pointer.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/spi/filter.h>

#include <memory>


namespace log4cplus {

    /**
     * This class is used to "handle" errors encountered in an {@link
     * log4cplus::Appender}.
     */
    class LOG4CPLUS_EXPORT ErrorHandler {
    public:
        virtual ~ErrorHandler();
        virtual void error(const log4cplus::tstring& err) = 0;
        virtual void reset() = 0;
    };



    class LOG4CPLUS_EXPORT OnlyOnceErrorHandler : public ErrorHandler,
                                                  protected log4cplus::helpers::LogLogUser
    {
    public:
      // Ctor
        OnlyOnceErrorHandler() : firstTime(true){}

        virtual void error(const log4cplus::tstring& err);
        virtual void reset();

    private:
        bool firstTime;
    };


    /**
     * Extend this class for implementing your own strategies for printing log
     * statements.
     */
    class LOG4CPLUS_EXPORT Appender
        : public virtual log4cplus::helpers::SharedObject
        , protected log4cplus::helpers::LogLogUser

    {
    public:
      // Ctor
        Appender();
        Appender(const log4cplus::helpers::Properties properties);

      // Dtor
        virtual ~Appender();

        void destructorImpl();

      // Methods
        /**
         * Release any resources allocated within the appender such as file
         * handles, network connections, etc.
         * 
         * It is a programming error to append to a closed appender.
         */
        virtual void close() = 0;

        /**
         * This method performs threshold checks and invokes filters before
         * delegating actual logging to the subclasses specific {@link
         * #append} method.
         */
        void doAppend(const log4cplus::spi::InternalLoggingEvent& event);

        /**
         * Get the name of this appender. The name uniquely identifies the
         * appender.
         */
        virtual log4cplus::tstring getName();

        /**
         * Set the name of this appender. The name is used by other
         * components to identify this appender.
         */
        virtual void setName(const log4cplus::tstring& name);

        /**
         * Set the {@link ErrorHandler} for this Appender.
         */
        virtual void setErrorHandler(std::auto_ptr<ErrorHandler> eh);

        /**
         * Return the currently set {@link ErrorHandler} for this
         * Appender.
         */
        virtual ErrorHandler* getErrorHandler();

        /**
         * Set the layout for this appender. Note that some appenders have
         * their own (fixed) layouts or do not use one. For example, the
         * SocketAppender ignores the layout set here.
         */
        virtual void setLayout(std::auto_ptr<Layout> layout);

        /**
         * Returns the layout of this appender. The value may be NULL.
         * 
         * This class owns the returned pointer.
         */
        virtual Layout* getLayout();

        /**
         * Set the filter chain on this Appender.
         */
        void setFilter(log4cplus::spi::FilterPtr f) { filter = f; }

        /**
         * Get the filter chain on this Appender.
         */
        log4cplus::spi::FilterPtr getFilter() const { return filter; }

        /**
         * Returns this appenders threshold LogLevel. See the {@link
         * #setThreshold} method for the meaning of this option.
         */
        LogLevel getThreshold() const { return threshold; }

        /**
         * Set the threshold LogLevel. All log events with lower LogLevel
         * than the threshold LogLevel are ignored by the appender.
         * 
         * In configuration files this option is specified by setting the
         * value of the <b>Threshold</b> option to a LogLevel
         * string, such as "DEBUG", "INFO" and so on.
         */
        void setThreshold(LogLevel th) { threshold = th; }

        /**
         * Check whether the message LogLevel is below the appender's
         * threshold. If there is no threshold set, then the return value is
         * always <code>true</code>.
         */
        bool isAsSevereAsThreshold(LogLevel ll) const {
            return ((ll != NOT_SET_LOG_LEVEL) && (ll >= threshold));
        }

    protected:
      // Methods
        /**
         * Subclasses of <code>Appender</code> should implement this
         * method to perform actual logging.
         * @see doAppend method.
         */
        virtual void append(const log4cplus::spi::InternalLoggingEvent& event) = 0;

      // Data
        /** The layout variable does not need to be set if the appender
         *  implementation has its own layout. */
        std::auto_ptr<Layout> layout;

        /** Appenders are named. */
        log4cplus::tstring name;

        /** There is no LogLevel threshold filtering by default.  */
        LogLevel threshold;

        /** The first filter in the filter chain. Set to <code>null</code>
         *  initially. */
        log4cplus::spi::FilterPtr filter;

        /** It is assumed and enforced that errorHandler is never null. */
        std::auto_ptr<ErrorHandler> errorHandler;

        /** Is this appender closed? */
        bool closed;
    };

    /** This is a pointer to an Appender. */
    typedef helpers::SharedObjectPtr<Appender> SharedAppenderPtr;

} // end namespace log4cplus

#endif // _LOG4CPLUS_APPENDER_HEADER_

