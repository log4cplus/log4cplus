// Module:  Log4CPLUS
// File:    appenderattachableimpl.h
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

#ifndef _LOG4CPLUS_HELPERS_APPENDER_ATTACHABLE_IMPL_HEADER_
#define _LOG4CPLUS_HELPERS_APPENDER_ATTACHABLE_IMPL_HEADER_

#include <log4cplus/config.hxx>
#include <log4cplus/layout.h>
#include <log4cplus/tstring.h>
#include <log4cplus/helpers/logloguser.h>
#include <log4cplus/helpers/pointer.h>
#include <log4cplus/helpers/threads.h>
#include <log4cplus/spi/appenderattachable.h>

#include <memory>
#include <vector>


namespace log4cplus {
    namespace helpers {
 
        /**
         * This Interface is for attaching Appenders to objects.
         */
        class LOG4CPLUS_EXPORT AppenderAttachableImpl 
                                   : public log4cplus::spi::AppenderAttachable,
                                     protected log4cplus::helpers::LogLogUser
        {
        public:
          // Data
            LOG4CPLUS_MUTEX_PTR_DECLARE appender_list_mutex;

          // Ctors
            AppenderAttachableImpl();

          // Dtor
            virtual ~AppenderAttachableImpl();

          // Methods
            /**
             * Add an appender.  If the appender is already in the list in
             * won't be added again.
             */
            virtual void addAppender(SharedAppenderPtr newAppender);

            /**
             * Get all previously added appenders as an vectory.
             */
            virtual SharedAppenderPtrList getAllAppenders();

            /**
             * Look for an attached appender named as <code>name</code>.
             *
             * Return the appender with that name if in the list. Return null
             * otherwise.  
             */
            virtual SharedAppenderPtr getAppender(const log4cplus::tstring& name);

            /**
             * Remove all previously added appenders.
             */
            virtual void removeAllAppenders();

            /**
             * Remove the appender passed as parameter from the list of appenders.
             */
            virtual void removeAppender(SharedAppenderPtr appender);

            /**
             * Remove the appender with the name passed as parameter from the
             * list of appenders.  
             */
            virtual void removeAppender(const log4cplus::tstring& name);

            /**
             * Call the <code>doAppend</code> method on all attached appenders.  
             */
            int appendLoopOnAppenders(const spi::InternalLoggingEvent& event) const;

        protected:
          // Types
            typedef std::vector<SharedAppenderPtr> ListType;

          // Data
            /** Array of appenders. */
            ListType appenderList;
        };  // end class AppenderAttachableImpl

    } // end namespace helpers
} // end namespace log4cplus

#endif // _LOG4CPLUS_HELPERS_APPENDER_ATTACHABLE_IMPL_HEADER_

