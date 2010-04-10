// Module:  Log4CPLUS
// File:    loglog.h
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

