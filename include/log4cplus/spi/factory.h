// Module:  Log4CPLUS
// File:    factory.h
// Created: 2/2002
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

#ifndef LOG4CPLUS_SPI_FACTORY_HEADER_
#define LOG4CPLUS_SPI_FACTORY_HEADER_

#include <log4cplus/config.h>
#include <log4cplus/appender.h>
#include <log4cplus/layout.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/helpers/threads.h>
#include <log4cplus/spi/objectregistry.h>
#include <map>
#include <memory>
#include <string>
#include <vector>


namespace log4cplus {
    namespace spi {

	/**
	 * This abstract class defines the "Factory" interface to create "Appender"
	 * objects.
	 */
        class AppenderFactory {
        public:
            AppenderFactory(){}
            virtual ~AppenderFactory(){}

	    /**
	     * Create an "Appender" object.
	     */
            virtual SharedAppenderPtr createObject(const log4cplus::helpers::Properties& props) = 0;

	    /**
	     * Returns the typename of the "Appender" objects this factory creates.
	     */
            virtual std::string getTypeName() = 0;
        };


	/**
	 * This abstract class defines the "Factory" interface to create "Layout"
	 * objects.
	 */
        class LayoutFactory {
        public:
            LayoutFactory(){}
            virtual ~LayoutFactory(){}

	    /**
	     * Create an "Layout" object.
	     */
            virtual std::auto_ptr<Layout> createObject(const log4cplus::helpers::Properties& props) = 0;

	    /**
	     * Returns the typename of the "Layout" objects this factory creates.
	     */
            virtual std::string getTypeName() = 0;
        };

        typedef ObjectRegistry<AppenderFactory> AppenderFactoryRegistry;
        typedef ObjectRegistry<LayoutFactory> LayoutFactoryRegistry;

	/**
	 * Returns the "singleton" <code>AppenderFactoryRegistry</code>.
	 */
        AppenderFactoryRegistry& getAppenderFactoryRegistry();

	/**
	 * Returns the "singleton" <code>LayoutFactoryRegistry</code>.
	 */
        LayoutFactoryRegistry& getLayoutFactoryRegistry();

    }
}


#endif // LOG4CPLUS_SPI_FACTORY_HEADER_

