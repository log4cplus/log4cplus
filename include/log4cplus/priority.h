// Module:  Log4CPLUS
// File:    priority.h
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

#ifndef LOG4CPLUS_PRIORITY_HEADER_
#define LOG4CPLUS_PRIORITY_HEADER_

#include <log4cplus/config.h>
#include <iostream>
#include <string>
#include <vector>

namespace log4cplus {

    // Forward Declarations
    class Priority;
    typedef std::vector<Priority> PriorityList;
    namespace spi {
        class CategoryImpl;
    }

    /**
     * Defines the minimum set of priorities recognized by the system,
     * that is {@link #FATAL_PRI}, {@link #ERROR_PRI}, {@link #WARN_PRI}, 
     * {@link #INFO_PRI} and {@link #DEBUG_PRI}.
     *
     * <p>The <code>Priority</code> class may be subclassed to define a larger
     * priority set.
     */
    class Priority {
    public:
      // Internal Types
        /** 
         * This is the default priority levels recognized by the system.
         */
        enum PriorityLevel {FATAL_PRI = 50000, /*!< The "Fatal" priority */ 
                            ERROR_PRI = 40000, /*!< The "Error" priority */
                            WARN_PRI  = 30000, /*!< The "Warn" priority  */ 
                            INFO_PRI  = 20000, /*!< The "Info" priority  */ 
                            DEBUG_PRI = 10000  /*!< The "Debug" priority */ 
                           };

      // Ctor
        explicit Priority(PriorityLevel level) : level(level) {}

      // Static Methods
        /**
         * Return all possible priorities as an array of Priority objects in
         * descending order.  
         */
        static PriorityList getAllPossiblePriorities();

        /**
         * Convert the string passed as argument to a priority. If the
         * conversion fails, then this method returns {@link #DEBUG_PRI}. 
         */
        static Priority toPriority(const std::string& sArg) 
            { return toPriority(sArg, Priority(DEBUG_PRI)); }
        
        /**
         * Convert an integer passed as argument to a priority. If the
         * conversion fails, then this method returns {@link #DEBUG_PRI}.
         */
        static Priority toPriority(int val) 
            { return toPriority(val, Priority(DEBUG_PRI)); }

        /**
         * Convert an integer passed as argument to a priority. If the
         * conversion fails, then this method returns the specified default.
         */
        static Priority toPriority(int val, const Priority& defaultPriority); 

        /**
         * Convert the string passed as argument to a priority. If the
         * conversion fails, then this method returns the value of
         * <code>defaultPriority</code>.  
         */
        static Priority toPriority(const std::string& sArg, 
                                   const Priority& defaultPriority);


      // Methods
        /**
         * Return the syslog equivalent of this priority as an integer.
         */
        virtual int getSyslogEquivalent() const;

        /**
         * Returns the string representation of this priority.
         */
        virtual std::string toString() const;

        /**
         * Returns the integer representation of this priority.
         */
        virtual int toInt() const { return level; };

        /**
         * Returns <code>true</code> if this priority has a higher or equal
         * priority than the priority passed as argument, <code>false</code>
         * otherwise.  
         *                     
         * <p>You should think twice before overriding the default
         * implementation of <code>isGreaterOrEqual</code> method.
         */
        bool isGreaterOrEqual(const Priority& p) const { return isGreaterOrEqual(&p); };
        virtual bool isGreaterOrEqual(const Priority* p) const;

      // Dtor
        virtual ~Priority(){}

    protected:
      // Data
        int level;

      // Friends
        friend class spi::CategoryImpl;
    };

};

inline std::ostream&
operator<<(std::ostream& OStream,
           const log4cplus::Priority& priority)
{
    return (OStream << priority.toString());
}


#endif // LOG4CPLUS_PRIORITY_HEADER_

