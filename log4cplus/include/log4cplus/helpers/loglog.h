// Module:  Log4CPLUS
// File:    loglog.h
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

#ifndef _LOG4CPLUS_HELPERS_LOGLOG
#define _LOG4CPLUS_HELPERS_LOGLOG

#include <log4cplus/config.h>
#include <log4cplus/helpers/threads.h>
#include <string>


namespace log4cplus {
    namespace helpers {
        // forward declarations
        class LogLog;

        /**
         * Returns a reference to the <code>LogLog</code> singleton.
         */
        LogLog& getLogLog();


        /**
         * This class used to output log statements from within the log4cplus package.
         *
         * <p>Log4cplus components cannot make log4cplus logging calls. However, it is
         * sometimes useful for the user to learn about what log4cplus is
         * doing. You can enable log4cplus internal logging by defining the
         * <b>log4cplus.configDebug</b> variable.
         *
         * <p>All log4cplus internal debug calls go to <code>cout</code>
         * where as internal error messages are sent to
         * <code>System.err</code>. All internal messages are prepended with
         * the string "log4clus: ".
         */
        class LogLog {
        public:
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
            void debug(const std::string& msg);

            /**
             * This method is used to output log4cplus internal error
             * statements. There is no way to disable error statements.
             * Output goes to <code>std::cerr</code>.
             */
            void error(const std::string& msg);

            /**
             * This method is used to output log4cplus internal warning
             * statements. There is no way to disable warning statements.
             * Output goes to <code>std::cerr</code>.
             */
            void warn(const std::string& msg);

          // Dtor
            ~LogLog();

          // Data
            LOG4CPLUS_MUTEX_PTR_DECLARE mutex;

        private:
          // Data
            bool debugEnabled;
            bool quietMode;
            const std::string PREFIX;
            const std::string WARN_PREFIX;
            const std::string ERR_PREFIX;

          // Ctors
            LogLog();
            LogLog(const LogLog&);

          // Friends
            friend LogLog& getLogLog();
        };

    }; // end namespace helpers
}; // end namespace log4cplus


#endif // _LOG4CPLUS_HELPERS_LOGLOG

