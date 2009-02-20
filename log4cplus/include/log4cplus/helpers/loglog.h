// Module:  Log4CPLUS
// File:    loglog.h
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

#ifndef _LOG4CPLUS_HELPERS_LOGLOG
#define _LOG4CPLUS_HELPERS_LOGLOG

#include <log4cplus/config.hxx>
#include <log4cplus/tstring.h>
#include <log4cplus/helpers/pointer.h>
#include <log4cplus/helpers/threads.h>


namespace log4cplus {
    namespace helpers {

        /**
         * This class used to output log statements from within the log4cplus package.
         *
         * Log4cplus components cannot make log4cplus logging calls. However, it is
         * sometimes useful for the user to learn about what log4cplus is
         * doing. You can enable log4cplus internal logging by defining the
         * <b>log4cplus.configDebug</b> variable.
         *
         * All log4cplus internal debug calls go to <code>cout</code>
         * where as internal error messages are sent to
         * <code>cerr</code>. All internal messages are prepended with
         * the string "log4clus: ".
         */
        class LOG4CPLUS_EXPORT LogLog
            : public virtual log4cplus::helpers::SharedObject 
        {
        public:
          // Static methods
            /**
             * Returns a reference to the <code>LogLog</code> singleton.
             */
            static log4cplus::helpers::SharedObjectPtr<LogLog> getLogLog();


            /**
             * Allows to enable/disable log4cplus internal logging.
             */
            void setInternalDebugging(bool enabled);

            /**
             * In quite mode no LogLog generates strictly no output, not even
             * for errors. 
             *
             * @param quietMode A true for not
             */
            void setQuietMode(bool quietMode);

            /**
             * This method is used to output log4cplus internal debug
             * statements. Output goes to <code>std::cout</code>.
             */
            void debug(const log4cplus::tstring& msg);

            /**
             * This method is used to output log4cplus internal error
             * statements. There is no way to disable error statements.
             * Output goes to <code>std::cerr</code>.
             */
            void error(const log4cplus::tstring& msg);

            /**
             * This method is used to output log4cplus internal warning
             * statements. There is no way to disable warning statements.
             * Output goes to <code>std::cerr</code>.
             */
            void warn(const log4cplus::tstring& msg);

          // Dtor
            virtual ~LogLog();

          // Data
            LOG4CPLUS_MUTEX_PTR_DECLARE mutex;

        private:
          // Data
            bool debugEnabled;
            bool quietMode;

          // Ctors
            LogLog();
            LogLog(const LogLog&);
        };

    } // end namespace helpers
} // end namespace log4cplus


#endif // _LOG4CPLUS_HELPERS_LOGLOG

