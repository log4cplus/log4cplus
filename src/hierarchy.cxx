// Module:  Log4CPLUS
// File:    hierarchy.cxx
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

#include <log4cplus/hierarchy.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/categoryimpl.h>
#include <log4cplus/spi/rootcategory.h>
#include <iostream>
#include <string>
#include <utility>

using namespace log4cplus;
using namespace log4cplus::helpers;

const int log4cplus::Hierarchy::DISABLE_OFF = -1;
const int log4cplus::Hierarchy::DISABLE_OVERRIDE = -2;



namespace {
    bool startsWith(std::string teststr, std::string substr) {
        bool val = false;
        if(teststr.length() > substr.length()) {
            val =  teststr.substr(0, substr.length()) == substr;
        }

        return val;
    }
}



Hierarchy::Hierarchy()
  : hashtable_mutex(LOG4CPLUS_MUTEX_CREATE),
    defaultFactory(new DefaultCategoryFactory()),
    root(NULL),
    disableValue(DISABLE_OFF),  // Don't disable any priority level by default.
    emittedNoAppenderWarning(false),
    emittedNoResourceBundleWarning(false)
{
    root = new Category( new spi::RootCategory(*this, Priority(Priority::DEBUG_PRI)) );
}


Hierarchy::~Hierarchy()
{
    shutdown();
    delete root;
    LOG4CPLUS_MUTEX_FREE( hashtable_mutex );
}


void 
Hierarchy::clear() 
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( hashtable_mutex )
        provisionNodes.erase(provisionNodes.begin(), provisionNodes.end());
        categoryPtrs.erase(categoryPtrs.begin(), categoryPtrs.end());
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}


bool
Hierarchy::exists(const std::string& name)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( hashtable_mutex )
        CategoryMap::iterator it = categoryPtrs.find(name);
        return it != categoryPtrs.end();
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}


void 
Hierarchy::disable(const std::string& priorityStr)
{
    if(disableValue != DISABLE_OVERRIDE) {
        disableValue = Priority::toPriority(priorityStr).toInt();
    }
}


void 
Hierarchy::disable(const Priority& p)
{
    if(disableValue != DISABLE_OVERRIDE) {
        disableValue = p.toInt();
    }
}


void 
Hierarchy::disableAll() 
{ 
    disable(Priority::FATAL_PRI); 
}


void 
Hierarchy::disableDebug() 
{ 
    disable(Priority::DEBUG_PRI); 
}


void 
Hierarchy::disableInfo() 
{ 
    disable(Priority::INFO_PRI); 
}


void 
Hierarchy::enableAll() 
{ 
    disableValue = DISABLE_OFF; 
}


Category 
Hierarchy::getInstance(const std::string& name) 
{ 
    return getInstance(name, *defaultFactory); 
}


Category 
Hierarchy::getInstance(const std::string& name, spi::CategoryFactory& factory)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( hashtable_mutex )
        CategoryMap::iterator it = categoryPtrs.find(name);
        if(it != categoryPtrs.end()) {
            return (*it).second;
        }
        else {
            // Need to create a new category
            Category cat = factory.makeNewCategoryInstance(name, *this);
            bool inserted = categoryPtrs.insert(std::make_pair(name, cat)).second;
            if(!inserted) {
                getLogLog().error("Hierarchy::getInstance()- Insert failed");
                throw std::runtime_error("Hierarchy::getInstance()- Insert failed");
            }

            ProvisionNodeMap::iterator it2 = provisionNodes.find(name);
            if(it2 != provisionNodes.end()) {
                updateChildren(it2->second, cat);
                bool deleted = provisionNodes.erase(name);
                if(!deleted) {
                    getLogLog().error("Hierarchy::getInstance()- Delete failed");
                    throw std::runtime_error("Hierarchy::getInstance()- Delete failed");
                }
            }
            updateParents(cat);

            return cat;
        }
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}


bool 
Hierarchy::isDisabled(int level) 
{ 
    return disableValue >= level; 
}


Category 
Hierarchy::getRoot() 
{ 
    return *root; 
}


CategoryList 
Hierarchy::getCurrentCategories()
{
    CategoryList ret;
 
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( hashtable_mutex )
        for(CategoryMap::iterator it=categoryPtrs.begin(); 
            it!= categoryPtrs.end(); 
            ++it) 
        {
            ret.push_back((*it).second);
        }
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX

    return ret;
}


void 
Hierarchy::resetConfiguration()
{
    getRoot().setPriority(Priority::DEBUG_PRI);
    disableValue = DISABLE_OFF;

    shutdown();

    CategoryList cats = getCurrentCategories();
    CategoryList::iterator it = cats.begin();
    while(it != cats.end()) {
        (*it).setPriority(NULL);
        (*it).setAdditivity(true);
        ++it;
    }

}


void 
Hierarchy::setCategoryFactory(std::auto_ptr<spi::CategoryFactory> factory) 
{ 
    defaultFactory = factory; 
}


void 
Hierarchy::shutdown()
{
    Category rootCat = getRoot();
    CategoryList cats = getCurrentCategories();

    // begin by closing nested appenders
    // then, remove all appenders
    rootCat.closeNestedAppenders();
    rootCat.removeAllAppenders();

    // repeat
    CategoryList::iterator it = cats.begin();
    while(it != cats.end()) {
        (*it).closeNestedAppenders();
        (*it).removeAllAppenders();
        ++it;
    }
}


void 
Hierarchy::updateParents(Category cat)
{
    std::string name = cat.getName();
    int length = name.length();
    bool parentFound = false;

    // if name = "w.x.y.z", loop thourgh "w.x.y", "w.x" and "w", but not "w.x.y.z"
    for(unsigned int i=name.find_last_of('.', length-1); 
        i != std::string::npos; 
        i = name.find_last_of('.', i-1)) 
    {
        std::string substr = name.substr(0, i);

        CategoryMap::iterator it = categoryPtrs.find(substr);
        if(it != categoryPtrs.end()) {
            parentFound = true;
            cat.value->parent = it->second.value;
            break;  // no need to update the ancestors of the closest ancestor
        }
        else {
            ProvisionNodeMap::iterator it2 = provisionNodes.find(substr);
            if(it2 != provisionNodes.end()) {
                it2->second.push_back(cat);
            }
            else {
                ProvisionNode node;
                node.push_back(cat);
                std::pair<ProvisionNodeMap::iterator, bool> tmp = 
                    provisionNodes.insert(std::make_pair(substr, node));
                //bool inserted = provisionNodes.insert(std::make_pair(substr, node)).second;
                if(!tmp.second) {
                    getLogLog().error("Hierarchy::updateParents()- Insert failed");
                    throw std::runtime_error("Hierarchy::updateParents()- Insert failed");
                }
            }
        } // end if Category found
    } // end for loop

    if(!parentFound) {
        cat.value->parent = root->value;
    }
}


void 
Hierarchy::updateChildren(ProvisionNode& pn, Category cat)
{

    for(ProvisionNode::iterator it=pn.begin(); it!=pn.end(); ++it) {
        Category& c = *it;
        // Unless this child already points to a correct (lower) parent,
        // make cat.parent point to c.parent and c.parent to cat.
        if( !startsWith(c.value->parent->getName(), cat.getName()) ) {
            cat.value->parent = c.value->parent;
            c.value->parent = cat.value;
        }
    }
}


