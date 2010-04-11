// Module:  Log4CPLUS
// File:    factory.h
// Created: 2/2002
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

#ifndef LOG4CPLUS_SPI_FACTORY_HEADER_
#define LOG4CPLUS_SPI_FACTORY_HEADER_

#include <log4cplus/config.h>
#include <log4cplus/appender.h>
#include <log4cplus/layout.h>
#include <log4cplus/tstring.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/helpers/threads.h>
#include <log4cplus/spi/filter.h>
#include <log4cplus/spi/objectregistry.h>
#include <map>
#include <memory>
#include <vector>


namespace log4cplus {
    namespace spi {

        /**
         * This is the base class for all factories.
         */
        class LOG4CPLUS_EXPORT BaseFactory {
        public:
            virtual ~BaseFactory() {}

            virtual log4cplus::tstring getTypeName() = 0;
        };


        /**
         * This abstract class defines the "Factory" interface to create "Appender"
         * objects.
         */
        class LOG4CPLUS_EXPORT AppenderFactory : public BaseFactory {
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
            virtual log4cplus::tstring getTypeName() = 0;
        };



        /**
         * This abstract class defines the "Factory" interface to create "Layout"
         * objects.
         */
        class LOG4CPLUS_EXPORT LayoutFactory : public BaseFactory {
        public:
            LayoutFactory(){}
            virtual ~LayoutFactory(){}

            /**
             * Create a "Layout" object.
             */
            virtual std::auto_ptr<Layout> createObject(const log4cplus::helpers::Properties& props) = 0;

            /**
             * Returns the typename of the "Layout" objects this factory creates.
             */
            virtual log4cplus::tstring getTypeName() = 0;
        };



        /**
         * This abstract class defines the "Factory" interface to create "Appender"
         * objects.
         */
        class LOG4CPLUS_EXPORT FilterFactory : public BaseFactory {
        public:
            FilterFactory(){}
            virtual ~FilterFactory(){}

            /**
             * Create a "Filter" object.
             */
            virtual FilterPtr createObject(const log4cplus::helpers::Properties& props) = 0;

            /**
             * Returns the typename of the "Filter" objects this factory creates.
             */
            virtual log4cplus::tstring getTypeName() = 0;
        };



        /**
         * This template class is used as a "Factory Registry".  Objects are
         * "entered" into the registry with a "name" using the 
         * <code>put()</code> method.  (The registry then owns the object.)  
         * These object can then be retrieved using the <code>get()</code> 
         * method.
         * <p>
         * <b>Note:</b>  This class is Thread-safe.
         */
        template<class T>
        class LOG4CPLUS_EXPORT FactoryRegistry : ObjectRegistryBase {
        public:
            virtual ~FactoryRegistry() {
                clear();
            }

          // public methods
            /**
             * Used to enter an object into the registry.  (The registry now
             *  owns <code>object</code>.)
             */
            bool put(std::auto_ptr<T> object) {
                 bool putValResult = putVal(object->getTypeName(), object.get());
                 object.release();
                 return putValResult; 
            }

            /**
             * Used to retrieve an object from the registry.  (The registry
             * owns the returned pointer.)
             */
            T* get(const log4cplus::tstring& name) const {
                return static_cast<T*>(getVal(name));
            }

        protected:
            virtual void deleteObject(void *object) const {
                delete static_cast<T*>(object);
            }
        };


        typedef FactoryRegistry<AppenderFactory> AppenderFactoryRegistry;
        typedef FactoryRegistry<LayoutFactory> LayoutFactoryRegistry;
        typedef FactoryRegistry<FilterFactory> FilterFactoryRegistry;


        /**
         * Returns the "singleton" <code>AppenderFactoryRegistry</code>.
         */
        LOG4CPLUS_EXPORT AppenderFactoryRegistry& getAppenderFactoryRegistry();

        /**
         * Returns the "singleton" <code>LayoutFactoryRegistry</code>.
         */
        LOG4CPLUS_EXPORT LayoutFactoryRegistry& getLayoutFactoryRegistry();

        /**
         * Returns the "singleton" <code>FilterFactoryRegistry</code>.
         */
        LOG4CPLUS_EXPORT FilterFactoryRegistry& getFilterFactoryRegistry();

    }
}


#endif // LOG4CPLUS_SPI_FACTORY_HEADER_

