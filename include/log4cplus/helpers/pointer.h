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


namespace log4cplus {
    namespace helpers {

#if (_MSC_VER >= 1300)
        // Added to remove the following warning from MSVC++ 7:
        // warning C4275: non dll-interface class 'std::runtime_error' used as
        //                base for dll-interface class
        //                'log4cplus::helpers::NullPointerException'
        class LOG4CPLUS_EXPORT std::runtime_error;
#endif

        class LOG4CPLUS_EXPORT NullPointerException
            : public std::runtime_error
        {
        public:
            NullPointerException(const std::string& what_arg)
                : std::runtime_error(what_arg)
            { }
        };

        void throwNullPointerException(const char* file, int line);


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
             : access_mutex(LOG4CPLUS_MUTEX_CREATE), count(0), destroyed(false) {}
            SharedObject(const SharedObject&)
             : access_mutex(LOG4CPLUS_MUTEX_CREATE), count(0), destroyed(false) {}

          // Dtor
            virtual ~SharedObject();

          // Operators
            SharedObject& operator=(const SharedObject&) { return *this; }

        public:
            LOG4CPLUS_MUTEX_PTR_DECLARE access_mutex;

        private:
            mutable int count;
            mutable bool destroyed;
        };


        /******************************************************************************
         *           Template Class SharedObjectPtr (from pp. 203, 206)               *
         ******************************************************************************/
        template<class T>
        class LOG4CPLUS_EXPORT SharedObjectPtr
        {
        public:
            // Ctor
            SharedObjectPtr(T* realPtr = 0)
                : pointee(realPtr)
            {
                init();
            }

            SharedObjectPtr(const SharedObjectPtr& rhs)
                : pointee(rhs.pointee)
            {
                init();
            }

            // Dtor
            ~SharedObjectPtr()
            {
                if (pointee)
                    static_cast<SharedObject *>(pointee)->removeReference();
            }

            // Operators
            bool operator==(const SharedObjectPtr& rhs) const { return (pointee == rhs.pointee); }
            bool operator!=(const SharedObjectPtr& rhs) const { return (pointee != rhs.pointee); }
            bool operator==(const T* rhs) const { return (pointee == rhs); }
            bool operator!=(const T* rhs) const { return (pointee != rhs); }
            T* operator->() const {validate(); return pointee; }
            T& operator*() const {validate(); return *pointee; }

            SharedObjectPtr& operator=(const SharedObjectPtr& rhs)
            {
                return this->operator = (rhs.pointee);
            }

            SharedObjectPtr& operator=(T* rhs)
            {
                if (pointee != rhs) {
                    T* oldPointee = pointee;
                    pointee = rhs;
                    init();
                    if(oldPointee != 0)
                        static_cast<SharedObject *>(oldPointee)->removeReference();
                }
                return *this;
            }

          // Methods
            T* get() const { return pointee; }

        private:
          // Methods
            void init() {
                if(pointee == 0) return;
                static_cast<SharedObject *>(pointee)->addReference();
            }
            void validate() const {
                if(pointee == 0) throw std::runtime_error("NullPointer");
            }

          // Data
            T* pointee;
        };

    } // end namespace helpers
} // end namespace log4cplus


#endif // _LOG4CPLUS_HELPERS_POINTERS_HEADER_
