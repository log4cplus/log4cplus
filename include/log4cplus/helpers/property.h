// Module:  Log4CPLUS
// File:    property.h
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

#ifndef LOG4CPLUS_HELPERS_PROPERTY_HEADER_
#define LOG4CPLUS_HELPERS_PROPERTY_HEADER_

#include <log4cplus/config.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>


namespace log4cplus {

    namespace helpers {
        class Properties {
        public:
            Properties();
            Properties(std::istream& input);
            Properties(const std::string& inputFile);
            virtual ~Properties();

          // methods
            /**
             * Tests to see if <code>key</code> can be found in this map.
             */
            bool exists(const std::string& key) const {
                return data.find(key) != data.end();
            }


            /**
             * Returns the number of entries in this map.
             */
            int size() const {
                return data.size();
            }

            /**
             * Searches for the property with the specified key in this property
             * list. If the key is not found in this property list, the default
             * property list, and its defaults, recursively, are then checked. 
             * The method returns <code>null</code> if the property is not found.
             */
            std::string getProperty(const std::string& key) const;

            /**
             * Searches for the property with the specified key in this property
             * list. If the key is not found in this property list, the default
             * property list, and its defaults, recursively, are then checked. 
             * The method returns the default value argument if the property is 
             * not found.
             */
            std::string getProperty(const std::string& key,
                                    const std::string& defaultVal) const;

            /**
             * Returns all the keys in this property list.
             */
            std::vector<std::string> propertyNames() const;

            /**
             * Inserts <code>value</code> into this map indexed by <code>key</code>.
             */
            void setProperty(const std::string& key, const std::string& value);


            /**
             * Returns a subset of the "properties" whose keys start with
             * "prefix".  The returned "properties" have "prefix" trimmed from
             * their keys.
             */
            Properties getPropertySubset(const std::string& prefix) const;

        protected:
          // Types
            typedef std::map<std::string, std::string> StringMap;

          // Methods
            void init(std::istream& input);

          // Data
            StringMap data;
        };
    } // end namespace helpers

}


#endif // LOG4CPLUS_HELPERS_PROPERTY_HEADER_

