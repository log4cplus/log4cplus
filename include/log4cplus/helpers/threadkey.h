// Module:  Log4CPLUS
// File:    threads.h
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

#ifndef _LOG4CPLUS_THREADKEYHEADER_
#define _LOG4CPLUS_THREADKEYHEADER_

#include <log4cplus/config.h>
#include <log4cplus/helpers/threads.h>
#include <map>


namespace log4cplus {
    namespace thread {

        /**
         * This class simulates a "ThreadKey".  Since it is not a true
         * "ThreadKey", then it will not automatically cleanup the memory
         * associated with that thread.
         *
         * This is intended to be used as a substitute for a "real" ThreadKey
         * implementation if the chosen threading library does not provide one.
         */
        class NonCleaningThreadKey {
        public:
            NonCleaningThreadKey() : mutex(LOG4CPLUS_MUTEX_CREATE) {}
            ~NonCleaningThreadKey(){ LOG4CPLUS_MUTEX_FREE(mutex); }

            /**
             * Retrieves the pointer set for this thread.
             */
            void* get();

            /**
             * Sets the pointer for this thread.
             */
            void set(void *arg);

            /**
             * Cleanup all internal structures allocated for the calling
             * thread.
             */
            void cleanEntry();

        private:
            typedef std::map<LOG4CPLUS_THREAD_KEY_TYPE, void*> MapType;
            LOG4CPLUS_MUTEX_PTR_DECLARE mutex;
            MapType map;
        };


    } // end namespace thread 
} // end namespace log4cplus 


#endif // _LOG4CPLUS_THREADKEYHEADER_

