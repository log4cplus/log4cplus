// Module:  Log4CPLUS
// File:    objectregistry.h
// Created: 3/2003
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

#ifndef LOG4CPLUS_SPI_OBJECT_REGISTRY_HEADER_
#define LOG4CPLUS_SPI_OBJECT_REGISTRY_HEADER_

#include <log4cplus/config.h>
#include <log4cplus/helpers/threads.h>
#include <map>
#include <memory>
#include <string>
#include <vector>


namespace log4cplus {
    namespace spi {

        /**
         * This template class is used as a "Object Registry".  Objects are
         * "entered" into the registry with a "name" using the <code>put()</code>
         * method.  (The registry then owns the object.)  These object can
         * then be retrieved using the <code>get()</code> method.
         * <p>
         * <b>Note:</b>  This class is Thread-safe.
         */
        template<class T>
        class ObjectRegistry {
        public:
          // Ctor and Dtor
            ObjectRegistry();
            ~ObjectRegistry();

          // public methods
            /**
             * Tests to see whether or not an object is bound in the
             * registry as <code>name</code>.
             */
            bool exists(const std::string& name) const;

            /**
             * Used the enter an object into the registry.  (The registry now
             * owns <code>object</code>.)
             */
            bool put(const std::string& name, std::auto_ptr<T> object);

            /**
             * Used to retrieve an object from the registry.  (The registry
             * owns the returned pointer.)
             */
            T* get(const std::string& name) const;

            /**
             * Returns the names of all registered objects.
             */
            std::vector<std::string> getAllNames() const;

        private:
          // Types
            typedef std::map<std::string, T*> ObjectMap;

          // Data
            LOG4CPLUS_MUTEX_PTR_DECLARE mutex;
            ObjectMap data;
        };


        // ------------------------------------------------------------
        // ObjectRegistry template implementation
        // ------------------------------------------------------------

        template<class T>
        ObjectRegistry<T>::ObjectRegistry()
         : mutex(LOG4CPLUS_MUTEX_CREATE)
        {
        }


        template<class T>
        ObjectRegistry<T>::~ObjectRegistry()
        {
            LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
            for(ObjectMap::iterator it=data.begin(); it!=data.end(); ++it) {
                delete (*it).second;
            }
            LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
            LOG4CPLUS_MUTEX_FREE( mutex );
        }


        template<class T>
        bool
        ObjectRegistry<T>::exists(const std::string& name) const
        {
            LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
                return data.find(name) != data.end();
            LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
        }


        template<class T>
        bool
        ObjectRegistry<T>::put(const string& name, auto_ptr<T> object)
        {
            LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
                ObjectMap::value_type value(name, object.release());
                pair<ObjectMap::iterator, bool> ret = data.insert(value);

                if(ret.second) {
                    return true;
                }
                else {
                    delete value.second;
                    return false;
                }
            LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
        }


        template<class T>
        T*
        ObjectRegistry<T>::get(const string& name) const
        {
            bool found = exists(name);
            LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
                if(found) {
                    return data.find(name)->second;
                }
                else {
                    return 0;
                }
            LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
        }


        template<class T>
        vector<string>
        ObjectRegistry<T>::getAllNames() const
        {
            LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
            vector<string> tmp;
                for(ObjectMap::const_iterator it=data.begin(); it!=data.end(); ++it) {
                    tmp.push_back( (*it).first );
                }

                return tmp;
            LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
        }


    }
}


#endif // LOG4CPLUS_SPI_OBJECT_REGISTRY_HEADER_

