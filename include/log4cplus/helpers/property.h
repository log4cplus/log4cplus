// Module:  Log4CPLUS
// File:    property.h
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

#ifndef LOG4CPLUS_HELPERS_PROPERTY_HEADER_
#define LOG4CPLUS_HELPERS_PROPERTY_HEADER_

#include <log4cplus/config.hxx>
#include <log4cplus/streams.h>
#include <log4cplus/tstring.h>
#include <map>
#include <vector>

#if (defined(__MWERKS__) && defined(__MACOS__))
using std::size_t;
#endif


namespace log4cplus {
    namespace helpers {

        class LOG4CPLUS_EXPORT Properties {
        public:
            Properties();
            explicit Properties(log4cplus::tistream& input);
            explicit Properties(const log4cplus::tstring& inputFile);
            virtual ~Properties();

          // constants
            static const tchar PROPERTIES_COMMENT_CHAR;

          // methods
            /**
             * Tests to see if <code>key</code> can be found in this map.
             */
            bool exists(const log4cplus::tstring& key) const {
                return data.find(key) != data.end();
            }


            /**
             * Returns the number of entries in this map.
             */
            size_t size() const {
                return data.size();
            }

            /**
             * Searches for the property with the specified key in this property
             * list. If the key is not found in this property list, the default
             * property list, and its defaults, recursively, are then checked. 
             * The method returns <code>null</code> if the property is not found.
             */
            log4cplus::tstring getProperty(const log4cplus::tstring& key) const;

            /**
             * Searches for the property with the specified key in this property
             * list. If the key is not found in this property list, the default
             * property list, and its defaults, recursively, are then checked. 
             * The method returns the default value argument if the property is 
             * not found.
             */
            log4cplus::tstring getProperty(const log4cplus::tstring& key,
                                           const log4cplus::tstring& defaultVal) const;

            /**
             * Returns all the keys in this property list.
             */
            std::vector<log4cplus::tstring> propertyNames() const;

            /**
             * Inserts <code>value</code> into this map indexed by <code>key</code>.
             */
            void setProperty(const log4cplus::tstring& key, const log4cplus::tstring& value);

            /**
             * Removed the property index by <code>key</code> from this map.
             */
            bool removeProperty(const log4cplus::tstring& key);

            /**
             * Returns a subset of the "properties" whose keys start with
             * "prefix".  The returned "properties" have "prefix" trimmed from
             * their keys.
             */
            Properties getPropertySubset(const log4cplus::tstring& prefix) const;

        protected:
          // Types
//            LOG4CPLUS_EXPIMP_TEMPLATE template class LOG4CPLUS_EXPORT std::map<log4cplus::tstring, log4cplus::tstring>;
            typedef std::map<log4cplus::tstring, log4cplus::tstring> StringMap;

          // Methods
            void init(log4cplus::tistream& input);

          // Data
            StringMap data;
        };
    } // end namespace helpers

}


#endif // LOG4CPLUS_HELPERS_PROPERTY_HEADER_

