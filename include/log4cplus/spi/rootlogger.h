// Module:  Log4CPLUS
// File:    rootlogger.h
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

#ifndef _LOG4CPLUS_SPI_ROOT_LOGGER_HEADER_
#define _LOG4CPLUS_SPI_ROOT_LOGGER_HEADER_

#include <log4cplus/config.hxx>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggerimpl.h>

namespace log4cplus {
    namespace spi {

        /**
         * RootLogger sits at the top of the logger hierachy. It is a
         * regular logger except that it provides several guarantees.
         *
         * First, it cannot be assigned a <code>NOT_SET_LOG_LEVEL</code>
         * LogLevel. Second, since root logger cannot have a parent, the
         * getChainedLogLevel method always returns the value of the
         * ll field without walking the hierarchy.
         */
        class LOG4CPLUS_EXPORT RootLogger : public LoggerImpl {
        public:
          // Ctors
            /**
             * The root logger names itself as "root". However, the root
             * logger cannot be retrieved by name.  
             */
            RootLogger(Hierarchy& h, LogLevel ll);

          // Methods
            /**
             * Return the assigned LogLevel value without walking the logger
             * hierarchy.
             */
            virtual LogLevel getChainedLogLevel() const;

            /**
             * Setting a NOT_SET_LOG_LEVEL value to the LogLevel of the root logger 
             * may have catastrophic results. We prevent this here.
             */
            void setLogLevel(LogLevel ll);

        };

    } // end namespace spi
} // end namespace log4cplus

#endif // _LOG4CPLUS_SPI_ROOT_LOGGER_HEADER_

