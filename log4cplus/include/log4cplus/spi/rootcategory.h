// Module:  Log4CPLUS
// File:    rootcategory.h
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

#ifndef _LOG4CPLUS_SPI_ROOT_CATEGORY_HEADER_
#define _LOG4CPLUS_SPI_ROOT_CATEGORY_HEADER_

#include "log4cplus/config.h"
#include "log4cplus/helpers/loglog.h"
#include "log4cplus/spi/categoryimpl.h"

using namespace log4cplus::helpers;

namespace log4cplus {
    namespace spi {

        /**
         * RootCategory sits at the top of the category hierachy. It is a
         * regular category except that it provides several guarantees.
         *
         * <p>First, it cannot be assigned a <code>null</code>
         * priority. Second, since root category cannot have a parent, the
         * {@link #getChainedPriority} method always returns the value of the
         * priority field without walking the hierarchy.
         */
        class RootCategory : public CategoryImpl {
        public:
        // Ctors
            /**
             * The root category names itself as "root". However, the root
             * category cannot be retrieved by name.  
             */
            RootCategory(Hierarchy& h, const Priority& priority) 
               : CategoryImpl(std::string("root"), h) { setPriority(&priority); }

        // Methods
            /**
             * Return the assigned priority value without walking the category
             * hierarchy.
             */
            const Priority& getChainedPriority() { return *priority; }

            /**
             * Setting a null value to the priority of the root category may have catastrophic
             * results. We prevent this here.
             */
            void setPriority(const Priority* priority) {
                if(priority == NULL) {
                    getLogLog().error("You have tried to set a NULL priority to root.");
                }
                else {
                    CategoryImpl::setPriority(priority); 
                }
            }

        };

    } // end namespace spi
} // end namespace log4cplus

#endif // _LOG4CPLUS_SPI_ROOT_CATEGORY_HEADER_

