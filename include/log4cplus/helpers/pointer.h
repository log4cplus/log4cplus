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

#include <log4cplus/config.h>
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

        class LOG4CPLUS_EXPORT NullPointerException : public std::runtime_error {
        public:
            NullPointerException(const std::string& what_arg) : std::runtime_error(what_arg) {}
        };

        void throwNullPointerException(const char* file, int line);

        template<class T>
        class LOG4CPLUS_EXPORT safe_auto_ptr {
        public:
          // Ctors
            explicit safe_auto_ptr(T* val = 0) : value(val){}
            safe_auto_ptr(const safe_auto_ptr& rhs) 
             : value(const_cast<safe_auto_ptr&>(rhs).value){}

            // Note: No Dtor needed since value is an auto_ptr

          // operators
            safe_auto_ptr& operator=(safe_auto_ptr& rhs) {value = rhs.value; return *this;}
            T& operator*() const { validate(); return *value; }
            T* operator->() const { validate(); return value.operator->(); }

          // methods
            T* get() const { return value.get(); }
            T* release() { return value.release(); }
            void reset(T* val = 0) { value.reset(val); }
            void validate(const char* file, int line) const { 
                if(value.get() == 0) {
                    throwNullPointerException(file, line);
                }
            }

        private:
            void validate() const { 
                if(value.get() == 0) {
                    throw NullPointerException("safe_auto_ptr::validate()- NullPointer");
                }
            }
            std::auto_ptr<T> value;
        };



        /******************************************************************************
         *                       Class SharedObject (from pp. 204-205)                *
         ******************************************************************************/

        class LOG4CPLUS_EXPORT SharedObject {
        public:
            void addReference();
            void removeReference();

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
            int count;
            bool destroyed;
        };


        /******************************************************************************
         *           Template Class SharedObjectPtr (from pp. 203, 206)               *
         ******************************************************************************/
        template<class T>
        class LOG4CPLUS_EXPORT SharedObjectPtr {
        public:
          // Ctor
            SharedObjectPtr(T* realPtr = 0) : pointee(realPtr) { init(); };
            SharedObjectPtr(const SharedObjectPtr& rhs) : pointee(rhs.pointee) { init(); };

          // Dtor
            ~SharedObjectPtr() {if (pointee != 0) ((SharedObject *)pointee)->removeReference(); }

          // Operators
            bool operator==(const SharedObjectPtr& rhs) const { return (pointee == rhs.pointee); }
            bool operator!=(const SharedObjectPtr& rhs) const { return (pointee != rhs.pointee); }
            bool operator==(const T* rhs) const { return (pointee == rhs); }
            bool operator!=(const T* rhs) const { return (pointee != rhs); }
            T* operator->() const {validate(); return pointee; }
            T& operator*() const {validate(); return *pointee; }
            SharedObjectPtr& operator=(const SharedObjectPtr& rhs) {
                if (pointee != rhs.pointee) {
                    T* oldPointee = pointee;
                    pointee = rhs.pointee;
                    init();
                    if(oldPointee != 0) oldPointee->removeReference();
                }
                return *this;
            }
            SharedObjectPtr& operator=(T* rhs) {
                if (pointee != rhs) {
                    T* oldPointee = pointee;
                    pointee = rhs;
                    init();
                    if(oldPointee != 0) oldPointee->removeReference();
                }
                return *this;
            }

          // Methods
            T* get() const { return pointee; }

        private:
          // Methods
            void init() {
                if(pointee == 0) return;
                ((SharedObject*)pointee)->addReference();
            }
            void validate() const {
                if(pointee == 0) throw std::runtime_error("NullPointer");
            }

          // Data
            T* pointee;
        };

    } // end namespace helpers
} // end namespace log4cplus

using log4cplus::helpers::safe_auto_ptr;

#endif // _LOG4CPLUS_HELPERS_POINTERS_HEADER_

