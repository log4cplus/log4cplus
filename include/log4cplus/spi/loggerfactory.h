// Module:  Log4CPLUS
// File:    loggerfactory.h
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

#ifndef _LOG4CPLUS_SPI_LOGGER_FACTORY_HEADER
#define _LOG4CPLUS_SPI_LOGGER_FACTORY_HEADER

#include <log4cplus/config.h>
#include <log4cplus/tstring.h>
#include <log4cplus/helpers/pointer.h>


namespace log4cplus {
    // Forward Declarations
    class Logger;
    class Hierarchy;

    namespace spi {
        /**
         * Implement this interface to create new instances of Logger or
         * a sub-class of Logger.
         */
        class LOG4CPLUS_EXPORT LoggerFactory {
        public:
            /**
             * Creates a new <code>Logger</code> object.
             */
            virtual Logger makeNewLoggerInstance(const log4cplus::tstring& name,
                                                 Hierarchy& h) = 0; 
            virtual ~LoggerFactory(){}
        };

    } // end namespace spi
} // end namespace log4cplus

#endif // _LOG4CPLUS_SPI_LOGGER_FACTORY_HEADER

