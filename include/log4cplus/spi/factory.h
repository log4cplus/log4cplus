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
#include <log4cplus/helpers/property.h>
#include <map>
#include <memory>
#include <string>
#include <vector>


namespace log4cplus {
    namespace spi {
        // Forward Declarations
        class FactoryRegistry;

        /**
         * Returns a reference to the <code>FactoryRegistry</code> singleton.
         */
        FactoryRegistry& getFactoryRegistry();

        class Factory {
        public:
            Factory(){}
            virtual ~Factory(){}

            virtual void* createObject(const std::string& configPrefix,
                                       const log4cplus::helpers::Properties& props) = 0;

            virtual std::string getTypeName() = 0;
        };


        
        class FactoryRegistry {
        public:
            bool registerFactory(const std::string& name,
                                 std::auto_ptr<Factory> factory);

            bool exists(const std::string& name) const;

            Factory* getFactory(const std::string& name) const;

            std::vector<std::string> getAllNames() const;

        private:
          // Ctor and Dtor
            FactoryRegistry();
            ~FactoryRegistry();

          // Types
            typedef std::map<std::string, Factory*> FactoryMap;

          // Data
            LOG4CPLUS_MUTEX_PTR_DECLARE mutex;
            FactoryMap data;

          // Friends
            friend FactoryRegistry& getFactoryRegistry();
        };

        

        

    }
}


#endif // LOG4CPLUS_SPI_FACTORY_HEADER_

