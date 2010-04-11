// Module:  Log4CPLUS
// File:    pointer.h
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

//
// Note: Some of this code uses ideas from "More Effective C++" by Scott
// Myers, Addison Wesley Longmain, Inc., (c) 1996, Chapter 29, pp. 183-213
//

/** @file */

#ifndef _LOG4CPLUS_HELPERS_POINTERS_HEADER_
#define _LOG4CPLUS_HELPERS_POINTERS_HEADER_

#include <log4cplus/config.hxx>
#include <memory>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <cassert>


namespace log4cplus {
    namespace helpers {

        /******************************************************************************
         *                       Class SharedObject (from pp. 204-205)                *
         ******************************************************************************/

        class LOG4CPLUS_EXPORT SharedObject
        {
        public:
            void addReference() const;
            void removeReference() const;

        protected:
          // Ctor
            SharedObject()
                : access_mutex(LOG4CPLUS_MUTEX_CREATE)
                , count(0)
            { }

            SharedObject(const SharedObject&)
                : access_mutex(LOG4CPLUS_MUTEX_CREATE)
                , count(0)
            { }

          // Dtor
            virtual ~SharedObject();

          // Operators
            SharedObject& operator=(const SharedObject&) { return *this; }

        public:
            LOG4CPLUS_MUTEX_PTR_DECLARE access_mutex;

        private:
            mutable int count;
        };


        /******************************************************************************
         *           Template Class SharedObjectPtr (from pp. 203, 206)               *
         ******************************************************************************/
        template<class T>
        class LOG4CPLUS_EXPORT SharedObjectPtr
        {
        public:
            // Ctor
            explicit
            SharedObjectPtr(T* realPtr = 0)
                : pointee(realPtr)
            {
                addref ();
            }

            SharedObjectPtr(const SharedObjectPtr& rhs)
                : pointee(rhs.pointee)
            {
                addref ();
            }

            // Dtor
            ~SharedObjectPtr()
            {
                if (pointee)
                    pointee->removeReference();
            }

            // Operators
            bool operator==(const SharedObjectPtr& rhs) const { return (pointee == rhs.pointee); }
            bool operator!=(const SharedObjectPtr& rhs) const { return (pointee != rhs.pointee); }
            bool operator==(const T* rhs) const { return (pointee == rhs); }
            bool operator!=(const T* rhs) const { return (pointee != rhs); }
            T* operator->() const {assert (pointee); return pointee; }
            T& operator*() const {assert (pointee); return *pointee; }

            SharedObjectPtr& operator=(const SharedObjectPtr& rhs)
            {
                return this->operator = (rhs.pointee);
            }

            SharedObjectPtr& operator=(T* rhs)
            {
                SharedObjectPtr (rhs).swap (*this);
                return *this;
            }

          // Methods
            T* get() const { return pointee; }

            void swap (SharedObjectPtr & other) throw ()
            {
                std::swap (pointee, other.pointee);
            }

            typedef T * (SharedObjectPtr:: * unspec_bool_type) () const;
            operator unspec_bool_type () const
            {
                return pointee ? &SharedObjectPtr::get : 0;
            }

            bool operator ! () const
            {
                return ! pointee;
            }

        private:
          // Methods
            void addref() const
            {
                if (pointee)
                    pointee->addReference();
            }

          // Data
            T* pointee;
        };

    } // end namespace helpers
} // end namespace log4cplus


#endif // _LOG4CPLUS_HELPERS_POINTERS_HEADER_
