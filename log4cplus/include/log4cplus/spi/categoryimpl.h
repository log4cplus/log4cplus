// Module:  Log4CPLUS
// File:    categoryimpl.h
// Created: 6/2001
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

#ifndef _LOG4CPLUS_SPI_CATEGORY_HEADER_
#define _LOG4CPLUS_SPI_CATEGORY_HEADER_

#include <log4cplus/config.h>
#include <log4cplus/category.h>
#include <log4cplus/helpers/appenderattachableimpl.h>
#include <log4cplus/helpers/pointer.h>
#include <log4cplus/spi/categoryfactory.h>
#include <memory>
#include <string>
#include <vector>


namespace log4cplus {
    namespace spi {

        // Forward declarations
        struct InternalLoggingEvent;

        /**
         * This is the central class in the log4j package. One of the
         * distintive features of log4j are hierarchical categories and their
         * evaluation.
         *
         * <p>See the <a href="../../../../manual.html">user manual</a> for an
         * introduction on this class. 
         */
        class CategoryImpl : public log4cplus::helpers::SharedObject,
                             public log4cplus::helpers::AppenderAttachableImpl
        {
        public:
          // Methods

            /**
             * Call the appenders in the hierrachy starting at
             * <code>this</code>.  If no appenders could be found, emit a
             * warning.
             * <p>
             * This method calls all the appenders inherited from the
             * hierarchy circumventing any evaluation of whether to log or not
             * to log the particular log request.
             *                                   
             * @param spi::InternalLoggingEvent the event to log. 
             */
            virtual void callAppenders(const InternalLoggingEvent& event);

            /**
             * Close all attached appenders implementing the AppenderAttachable
             * interface.  
             */
            virtual void closeNestedAppenders();

            /**
             * Check whether this category is enabled for a given {@link
             * Priority} passed as parameter.
             *
             * See also {@link #isDebugEnabled}.
             *                      
             * @return boolean True if this category is enabled for <code>priority</code>.
             */
            virtual bool isEnabledFor(const Priority& priority) const;
            bool isEnabledFor(int priority) const { 
                return isEnabledFor( Priority(static_cast<Priority::PriorityLevel>(priority))); 
            }

            /**
             * This generic form is intended to be used by wrappers. 
             */
            virtual void log(const Priority& priority, const std::string& message,
                             const char* file=NULL, int line=-1);

            /**
             * Starting from this category, search the category hierarchy for a
             * non-NULL priority and return it. Otherwise, return the priority of the
             * root category.
             *                     
             * <p>The Category class is designed so that this method executes as
             * quickly as possible.
             */
            virtual Priority getChainedPriority() const;

            /**
             * Returns the assigned {@link Priority}, if any, for this Category.  
             *           
             * @return Priority - the assigned Priority, can be <code>NULL</code>.
             */
            const Priority* getPriority() const { return this->priority; }

            /**
             * Set the priority of this Category.
             *
             * <p>NULL values are admitted.
             */
            virtual void setPriority(const Priority* priority);

            /**
             * Return the the {@link Hierarchy} where this <code>Category</code>
             * instance is attached.
             */
            virtual Hierarchy& getHierarchy() const;

            /**
             * Return the category name.  
             */
            std::string getName() const { return name; }

            /**
             * Get the additivity flag for this Category instance.
             */
            bool getAdditivity() const;

            /**
             * Set the additivity flag for this Category instance.
             */
            void setAdditivity(bool additive);

            virtual ~CategoryImpl();

        protected:
          // Ctors
            /**
             * This constructor created a new <code>Category</code> instance and
             * sets its name.
             *
             * <p>It is intended to be used by sub-classes only. You should not
             * create categories directly.
             *
             * @param name The name of the category.  
             */
            CategoryImpl(const std::string& name, Hierarchy& h);


          // Methods
            /**
             * This method creates a new logging event and logs the event
             * without further checks.  
             */
            virtual void forcedLog(const Priority& priority, 
                                   const std::string& message,
                                   const char* file=NULL, 
                                   int line=-1);


          // Data
            /** The name of this category */
            std::string name;

            /**
             * The assigned priority of this category.  The
             * <code>priority</code> variable need not be assined a value in
             * which case it is inherited from the hierarchy.  
             */
            const Priority* priority;

            /**
             * The parent of this category. All categories have at least one
             * ancestor which is the root category. 
             */ 
            SharedCategoryImplPtr parent;

            /** 
             * Additivity is set to true by default, that is children inherit
             * the appenders of their ancestors by default. If this variable is
             * set to <code>false</code> then the appenders found in the
             * ancestors of this category are not used. However, the children
             * of this category will inherit its appenders, unless the children
             * have their additivity flag set to <code>false</code> too. See
             * the user manual for more details. 
             */
            bool additive;

        private:
          // Data
            /** Categories need to know what Hierarchy they are in. */
            Hierarchy& hierarchy;

          // Friends
            friend class Category;
            friend class DefaultCategoryFactory;
            friend class Hierarchy;
        };

    } // end namespace spi
} // end namespace log4cplus

#endif // _LOG4CPLUS_SPI_CATEGORY_HEADER_


