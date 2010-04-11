// Module:  Log4CPLUS
// File:    loglevel.h
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

/** @file 
 * This header defines the LogLevel type.
 */

#ifndef LOG4CPLUS_LOGLEVEL_HEADER_
#define LOG4CPLUS_LOGLEVEL_HEADER_

#include <log4cplus/config.h>
#include <log4cplus/streams.h>
#include <log4cplus/tstring.h>

#include <vector>

namespace log4cplus {

    /** 
     * \typedef int LogLevel
     * Defines the minimum set of priorities recognized by the system,
     * that is {@link #FATAL_LOG_LEVEL}, {@link #ERROR_LOG_LEVEL}, {@link
     * #WARN_LOG_LEVEL}, {@link #INFO_LOG_LEVEL}, {@link #DEBUG_LOG_LEVEL}, 
     * and {@link #TRACE_LOG_LEVEL}.
     */
    typedef int LogLevel;

    /** \var const LogLevel OFF_LOG_LEVEL
     * The <code>OFF_LOG_LEVEL</code> LogLevel is used during configuration to
     * turn off logging. */
    const LogLevel OFF_LOG_LEVEL     = 60000;

    /** \var const LogLevel FATAL_LOG_LEVEL
     * The <code>FATAL_LOG_LEVEL</code> LogLevel designates very severe error
     * events that will presumably lead the application to abort.  */
    const LogLevel FATAL_LOG_LEVEL   = 50000;

    /** \var const LogLevel ERROR_LOG_LEVEL
     * The <code>ERROR_LOG_LEVEL</code> LogLevel designates error events that
     * might still allow the application to continue running. */
    const LogLevel ERROR_LOG_LEVEL   = 40000;

    /** \var const LogLevel WARN_LOG_LEVEL
     * The <code>WARN_LOG_LEVEL</code> LogLevel designates potentially harmful 
     * situations. */
    const LogLevel WARN_LOG_LEVEL    = 30000;

    /** \var const LogLevel INFO_LOG_LEVEL
     * The <code>INFO_LOG_LEVEL</code> LogLevel designates informational 
     * messages  that highlight the progress of the application at 
     * coarse-grained  level. */
    const LogLevel INFO_LOG_LEVEL    = 20000;

    /** \var const LogLevel DEBUG_LOG_LEVEL
     * The <code>DEBUG_LOG_LEVEL</code> LogLevel designates fine-grained
     * informational events that are most useful to debug an application. */
    const LogLevel DEBUG_LOG_LEVEL   = 10000;

    /** \var const LogLevel TRACE_LOG_LEVEL
     * The <code>TRACE_LOG_LEVEL</code> LogLevel is used to "trace" entry
     * and exiting of methods. */
    const LogLevel TRACE_LOG_LEVEL   = 0;
    
    /** \var const LogLevel ALL_LOG_LEVEL
     * The <code>ALL_LOG_LEVEL</code> LogLevel is used during configuration to
     * turn on all logging. */
    const LogLevel ALL_LOG_LEVEL     = TRACE_LOG_LEVEL;

    /** \var const LogLevel NOT_SET_LOG_LEVEL
     * The <code>NOT_SET_LOG_LEVEL</code> LogLevel is used to indicated that
     * no particular LogLevel is desired and that the default should be used.
     */
    const LogLevel NOT_SET_LOG_LEVEL = -1;


    /** 
     * This method type defined the signature of methods that convert LogLevels
     * into strings. 
     * <p>
     * <b>Note:</b> Must return an empty <code>tstring</code> for unrecognized values.
     */
    typedef log4cplus::tstring (*LogLevelToStringMethod)(LogLevel);

    /** 
     * This method type defined the signature of methods that convert strings
     * into LogLevels. 
     * <p>
     * <b>Note:</b> Must return <code>NOT_SET_LOG_LEVEL</code> for unrecognized values.
     */
    typedef LogLevel (*StringToLogLevelMethod)(const log4cplus::tstring&);

    

    /**
     * This class is used to "manage" LogLevel definitions.  This class is also
     * how "derived" LogLevels are created. Here are the steps to creating a
     * "derived" LogLevel:
     * <ol>
     *   <li>Create a LogLevel constant (greater than 0)</li>
     *   <li>Define a string to represent that constant</li>
     *   <li>Implement a LogLevelToStringMethod method.</li>
     *   <li>Implement a StringToLogLevelMethod method.</li>
     *   <li>create a "static initializer" that registers those 2 methods
     *       with the LogLevelManager singleton.</li>
     * </ol>
     */
    class LOG4CPLUS_EXPORT LogLevelManager {
    public:
        LogLevelManager();
        ~LogLevelManager();

        /**
         * This method is called by all Layout classes to convert a LogLevel
         * into a string.
         * <p>
         * Note: It traverses the list of <code>LogLevelToStringMethod</code>
         *       to do this, so all "derived" LogLevels are recognized as well.
         */
        log4cplus::tstring toString(LogLevel ll) const;
        
        /**
         * This method is called by all classes internally to log4cplus to
         * convert a string into a LogLevel.
         * <p>
         * Note: It traverses the list of <code>StringToLogLevelMethod</code>
         *       to do this, so all "derived" LogLevels are recognized as well.
         */
        LogLevel fromString(const log4cplus::tstring& s) const;

        /**
         * When creating a "derived" LogLevel, a <code>LogLevelToStringMethod</code>
         * should be defined and registered with the LogLevelManager by calling
         * this method.
         * <p>
         * @see pushFromStringMethod
         */
        void pushToStringMethod(LogLevelToStringMethod newToString);

        /**
         * When creating a "derived" LogLevel, a <code>StringToLogLevelMethod</code>
         * should be defined and registered with the LogLevelManager by calling
         * this method.
         * <p>
         * @see pushToStringMethod
         */
        void pushFromStringMethod(StringToLogLevelMethod newFromString);

    private:
      // Data
        void* toStringMethods;
        void* fromStringMethods;

      // Disable Copy
        LogLevelManager(const LogLevelManager&);
	LogLevelManager& operator=(const LogLevelManager&);
    };

    /**
     * Returns the singleton LogLevelManager.
     */
    LOG4CPLUS_EXPORT LogLevelManager& getLogLevelManager();

}


#endif // LOG4CPLUS_LOGLEVEL_HEADER_

