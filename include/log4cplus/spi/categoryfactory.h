// Module:  Log4CPLUS
// File:    categoryfactory.h
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

#ifndef _LOG4CPLUS_SPI_
#define _LOG4CPLUS_SPI_

#include <log4cplus/config.h>
#include <log4cplus/helpers/pointer.h>
#include <string>


namespace log4cplus {
    // Forward Declarations
    class Category;
    class Hierarchy;

    namespace spi {
        /**
         * Implement this interface to create new instances of Category or
         * a sub-class of Category.
         */
        class CategoryFactory {
        public:
            /**
             * Creates a new <code>Category</code> object.
             */
            virtual Category makeNewCategoryInstance(const std::string& name,
                                                     Hierarchy& h) = 0; 
            virtual ~CategoryFactory(){}
        };

    }; // end namespace spi
}; // end namespace log4cplus

#endif // _LOG4CPLUS_SPI_

