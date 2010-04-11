// Module:  Log4CPLUS
// File:    hierarchylocker.h
// Created: 8/2003
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

#ifndef _LOG4CPLUS_HIERARCHY_LOCKER_HEADER_
#define _LOG4CPLUS_HIERARCHY_LOCKER_HEADER_

#include <log4cplus/hierarchy.h>


namespace log4cplus {

    /**
     * This is used to lock a Hierarchy.  The dtor unlocks the Hierarchy.
     */
    class LOG4CPLUS_EXPORT HierarchyLocker {
    public:
      // ctor & dtor
        HierarchyLocker(Hierarchy& h);
        ~HierarchyLocker();
        
        /**
         * Calls the <code>resetConfiguration()</code> method on the locked Hierarchy.
         */
        void resetConfiguration(); 
        
        /**
         * Calls the <code>getInstance()</code> method on the locked Hierarchy.
         */
        Logger getInstance(const log4cplus::tstring& name);
        
        /**
         * Calls the <code>getInstance()</code> method on the locked Hierarchy.
         */
        Logger getInstance(const log4cplus::tstring& name, spi::LoggerFactory& factory);
        
        void addAppender(Logger &logger, log4cplus::SharedAppenderPtr& appender);
        
    private:
      // Data
        Hierarchy& h;
        log4cplus::thread::Guard hierarchyLocker;
        LoggerList loggerList;
    };

} // end namespace log4cplus

#endif // _LOG4CPLUS_HIERARCHY_LOCKER_HEADER_

