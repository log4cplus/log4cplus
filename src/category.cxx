// Module:  Log4CPLUS
// File:    category.cxx
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

#include <log4cplus/category.h>
#include <log4cplus/appender.h>
#include <log4cplus/hierarchy.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/rootcategory.h>
#include <stdexcept>

using namespace log4cplus;
using namespace log4cplus::helpers;


Category 
DefaultCategoryFactory::makeNewCategoryInstance(const std::string& name, Hierarchy& h)
{ 
    return Category( new spi::CategoryImpl(name, h) );
}


//////////////////////////////////////////////////////////////////////////////
// static Category Methods
//////////////////////////////////////////////////////////////////////////////
Hierarchy& 
Category::getDefaultHierarchy()
{
    static Hierarchy defaultHierarchy;

    return defaultHierarchy;
}


bool 
Category::exists(const std::string& name) 
{
    return getDefaultHierarchy().exists(name); 
}


CategoryList
Category::getCurrentCategories() 
{
    return getDefaultHierarchy().getCurrentCategories();
}


Category 
Category::getInstance(const std::string& name) 
{ 
    return getDefaultHierarchy().getInstance(name); 
}


Category 
Category::getInstance(const std::string& name, spi::CategoryFactory& factory)
{ 
    return getDefaultHierarchy().getInstance(name, factory); 
}


Category 
Category::getRoot() 
{ 
    return getDefaultHierarchy().getRoot(); 
}



void 
Category::shutdown() 
{ 
    getDefaultHierarchy().shutdown(); 
}



//////////////////////////////////////////////////////////////////////////////
// Category Constructors and Destructor
//////////////////////////////////////////////////////////////////////////////
Category::Category(const spi::SharedCategoryImplPtr& val)
 : value(val.get())
{
    init();
}


Category::Category(spi::CategoryImpl *ptr)
 : value(ptr)
{
    init();
}


Category::Category(const Category& rhs)
 : value(rhs.value)
{
    init();
}


Category&
Category::operator=(const Category& rhs)
{
    if (value != rhs.value) {
        spi::CategoryImpl *oldValue = value;

        value = rhs.value;
        init();
        if(oldValue != NULL) {
            oldValue->removeReference();
        }
    }

    return *this;
}


Category::~Category() 
{
    if (value != NULL) {
        value->removeReference();
    }
}


Category
Category::getParent() {
    validate(__FILE__, __LINE__);
    if(value->parent.get() != NULL) {
        return Category(value->parent);
    }
    else {
        getLogLog().error("********* This category has no parent: " + getName());
        return *this;
    }
}



//////////////////////////////////////////////////////////////////////////////
// Category Methods
//////////////////////////////////////////////////////////////////////////////
void
Category::init() {
    if(value == NULL) return;
    value->addReference();
}


void
Category::validate(const char *file, int line) const
{
    if (value == NULL) {
        getLogLog().error("Category::validate()- Internal log4cplus error: NullPointerException");
        log4cplus::helpers::throwNullPointerException(file, line);
    }
}



void 
Category::callAppenders(const spi::InternalLoggingEvent& event)
{
    validate(__FILE__, __LINE__);
    value->callAppenders(event);
}


void 
Category::closeNestedAppenders()
{
    validate(__FILE__, __LINE__);
    value->closeNestedAppenders();
}


bool 
Category::isEnabledFor(const Priority& priority) const
{
    validate(__FILE__, __LINE__);
    return value->isEnabledFor(priority);
}


void 
Category::log(const Priority& priority, const std::string& message,
              const char* file, int line)
{
    validate(__FILE__, __LINE__);
    value->log(priority, message, file, line);
}


Priority
Category::getChainedPriority() const
{
    validate(__FILE__, __LINE__);
    return value->getChainedPriority();
}


const Priority* 
Category::getPriority() const
{
    validate(__FILE__, __LINE__);
    return value->getPriority();
}


void 
Category::setPriority(const Priority* priority)
{
    validate(__FILE__, __LINE__);
    value->setPriority(priority);
}


Hierarchy& 
Category::getHierarchy() const
{ 
    validate(__FILE__, __LINE__);
    return value->getHierarchy();
}


std::string 
Category::getName() const
{
    validate(__FILE__, __LINE__);
    return value->getName();
}


bool 
Category::getAdditivity() const
{
    validate(__FILE__, __LINE__);
    return value->getAdditivity();
}


void 
Category::setAdditivity(bool additive) 
{ 
    validate(__FILE__, __LINE__);
    value->setAdditivity(additive);
}


void 
Category::addAppender(SharedAppenderPtr newAppender)
{
    validate(__FILE__, __LINE__);
    value->addAppender(newAppender);
}


SharedAppenderPtrList 
Category::getAllAppenders()
{
    validate(__FILE__, __LINE__);
    return value->getAllAppenders();
}


SharedAppenderPtr 
Category::getAppender(const std::string& name)
{
    validate(__FILE__, __LINE__);
    return value->getAppender(name);
}


void 
Category::removeAllAppenders()
{
    validate(__FILE__, __LINE__);
    value->removeAllAppenders();
}


void 
Category::removeAppender(SharedAppenderPtr appender)
{
    validate(__FILE__, __LINE__);
    value->removeAppender(appender);
}


void 
Category::removeAppender(const std::string& name)
{
    validate(__FILE__, __LINE__);
    value->removeAppender(name);
}


void 
Category::forcedLog(const Priority& priority, const std::string& message,
                    const char* file, int line)
{
    validate(__FILE__, __LINE__);
    value->forcedLog(priority, message, file, line);
}


