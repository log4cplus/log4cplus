// Module:  Log4CPLUS
// File:    appender.h
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright (C) The Apache Software Foundation. All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

/** @file */

#ifndef _LOG4CPLUS_APPENDER_HEADER_
#define _LOG4CPLUS_APPENDER_HEADER_

#include <log4cplus/config.h>
#include <log4cplus/layout.h>
#include <log4cplus/priority.h>
#include <log4cplus/helpers/pointer.h>
#include <log4cplus/helpers/property.h>
#include <memory>
#include <string>


namespace log4cplus {

    // Forward Declarations
    namespace spi {
        struct InternalLoggingEvent;
    }



    /**
     * This class is used to "handle" errors encountered in an {@link
     * #Appender}.
     */
    class ErrorHandler {
    public:
        virtual ~ErrorHandler();
        virtual void error(const std::string& err) = 0;
    };



    class OnlyOnceErrorHandler : public ErrorHandler {
    public:
      // Ctor
        OnlyOnceErrorHandler() : firstTime(true){}

        virtual void error(const std::string& err);

    private:
        bool firstTime;
    };


    /**
     * Extend this class for implementing your own strategies for printing log
     * statements. 
     */
    class Appender : public log4cplus::helpers::SharedObject {
    public:
      // Ctor
        Appender();
        Appender(log4cplus::helpers::Properties properties);

      // Dtor
        virtual ~Appender(){};

        void destructorImpl();

      // Methods
        /**
         * Release any resources allocated within the appender such as file
         * handles, network connections, etc.
         * <p>
         * It is a programming error to append to a closed appender.
         */
        virtual void close() = 0;

        /**
         * This method performs threshold checks and invokes filters before
         * delegating actual logging to the subclasses specific {@link
         * AppenderSkeleton#append} method.
         */
        virtual void doAppend(const log4cplus::spi::InternalLoggingEvent& event);

        /**
         * Get the name of this appender. The name uniquely identifies the
         * appender.  
         */
        virtual std::string getName();

        /**
         * Set the name of this appender. The name is used by other
         * components to identify this appender.
         */
        virtual void setName(const std::string& name);

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
         * {@link org.apache.log4j.net.SocketAppender} ignores the layout set
         * here. 
         */
        virtual void setLayout(std::auto_ptr<Layout> layout);

        /**
         * Returns the layout of this appender. The value may be NULL.
         * <p>
         * This class owns the returned pointer.
         */
        virtual Layout* getLayout();

        /**
         * Returns this appenders threshold priority. See the {@link
         * #setThreshold} method for the meaning of this option.
         */
        const Priority* getThreshold();

        /**
         * Set the threshold priority. All log events with lower priority
         * than the threshold priority are ignored by the appender.
         * <p>
         * In configuration files this option is specified by setting the
         * value of the <b>Threshold</b> option to a priority
         * string, such as "DEBUG", "INFO" and so on.
         */
        void setThreshold(const Priority* threshold);

        /**
         * Check whether the message priority is below the appender's
         * threshold. If there is no threshold set, then the return value is
         * always <code>true</code>.
         */
        virtual bool isAsSevereAsThreshold(const Priority* priority);
        bool isAsSevereAsThreshold(const Priority& priority) 
            {return isAsSevereAsThreshold(&priority); }


    protected:
      // Methods
        /**
         * Subclasses of <code>AppenderSkeleton</code> should implement this
         * method to perform actual logging. See also {@link #doAppend
         * AppenderSkeleton.doAppend} method.
         */
        virtual void append(const log4cplus::spi::InternalLoggingEvent& event) = 0;

      // Data
        /** The layout variable does not need to be set if the appender
         *  implementation has its own layout. */
        std::auto_ptr<Layout> layout;

        /** Appenders are named. */
        std::string name;

        /** There is no priority threshold filtering by default.  */
        const Priority* threshold;

        /** It is assumed and enforced that errorHandler is never null. */
        std::auto_ptr<ErrorHandler> errorHandler;

        /** Is this appender closed? */
        bool closed;
    };

    /** @var This is a pointer to an Appender. */
    typedef helpers::SharedObjectPtr<Appender> SharedAppenderPtr;

}; // end namespace log4cplus

#endif // _LOG4CPLUS_APPENDER_HEADER_

