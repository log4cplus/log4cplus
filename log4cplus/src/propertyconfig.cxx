// Module:  Log4CPLUS
// File:    propertyconfig.cxx
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

#include <log4cplus/propertyconfig.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/spi/factory.h>

#include <algorithm>

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::spi;


//////////////////////////////////////////////////////////////////////////////
// log4cplus::PropertyConfigurator ctor and dtor
//////////////////////////////////////////////////////////////////////////////

log4cplus::PropertyConfigurator::PropertyConfigurator(const std::string& propertyFile)
: propertyFilename(propertyFile),
  properties(propertyFile)
{
    properties = properties.getPropertySubset("log4cplus.");
}



log4cplus::PropertyConfigurator::~PropertyConfigurator()
{
}



//////////////////////////////////////////////////////////////////////////////
// log4cplus::PropertyConfigurator public methods
//////////////////////////////////////////////////////////////////////////////

void
log4cplus::PropertyConfigurator::configure()
{
    configureAppenders();
    configureCategories();
    configureAdditivity();
}



//////////////////////////////////////////////////////////////////////////////
// log4cplus::PropertyConfigurator protected methods
//////////////////////////////////////////////////////////////////////////////

void
log4cplus::PropertyConfigurator::configureCategories()
{
    if(properties.exists("rootCategory")) {
        Category rootCat = Category::getRoot();
        configureCategory(rootCat, properties.getProperty("rootCategory"));
    }

    Properties categoryProperties = properties.getPropertySubset("category.");
    vector<string> categories = categoryProperties.propertyNames();
    for(vector<string>::iterator it=categories.begin(); it!=categories.end(); ++it) {
        Category cat = Category::getInstance(*it);
        configureCategory(cat, categoryProperties.getProperty(*it));
    }
}



void
log4cplus::PropertyConfigurator::configureCategory(log4cplus::Category cat, 
                                                   const std::string& config)
{
    // Remove all spaces from config
    string configString;
    remove_copy_if(config.begin(), config.end(),
                   back_insert_iterator<string>(configString),
                   bind1st(equal_to<char>(), ' '));

    // "Tokenize" configString
    vector<string> tokens;
    tokenize(configString, ',',
             back_insert_iterator<vector<string> >(tokens));

    if(tokens.size() == 0) {
        getLogLog().error("PropertyConfigurator::configureCategory()- Invalid config " \
                          "string(Category = " + cat.getName() + "): \"" + config + "\"");
        return;
    }

    // Set the priority
    string priority = tokens[0];
    if(priority != "INHERITED") {
        cat.setPriority( static_cast<Priority::PriorityLevel>(Priority::toPriority(priority).toInt()) );
    }

    // Set the Appenders
    for(int j=1; j<tokens.size(); ++j) {
        AppenderMap::iterator appenderIt = appenders.find(tokens[j]);
        if(appenderIt == appenders.end()) {
            getLogLog().error("PropertyConfigurator::configureCategory()- Invalid " \
                              "appender: " + tokens[j]);
            continue;
        }
        cat.addAppender( (*appenderIt).second );
    }
}



void
log4cplus::PropertyConfigurator::configureAppenders()
{
    Properties appenderProperties = properties.getPropertySubset("appender.");
    vector<string> appendersProps = appenderProperties.propertyNames();
    for(vector<string>::iterator it=appendersProps.begin(); it!=appendersProps.end(); ++it) {
        if( (*it).find('.') == string::npos ) {
            string factoryName = appenderProperties.getProperty(*it);
            AppenderFactory* factory = getAppenderFactoryRegistry().get(factoryName);
            if(factory == 0) {
                getLogLog().error("PropertyConfigurator::configureAppenders()- Cannot " \
                                  "find AppenderFactory: " + factoryName);
                continue;
            }

            Properties properties = appenderProperties.getPropertySubset( (*it) + "." );
            try {
                SharedAppenderPtr appender = factory->createObject(properties);
                if(appender.get() == 0) {
                    getLogLog().error("PropertyConfigurator::configureAppenders()- Failed " \
                                      "to create appender: " + *it);
                }
                else {
                    appender->setName(*it);
                    appenders[*it] = appender;
                }
            }
            catch(std::exception& e) {
                getLogLog().error("PropertyConfigurator::configureAppenders()- Error " \
                                  "while creating Appender: " + string(e.what()));
            }
        }
    } // end for loop
}


void
log4cplus::PropertyConfigurator::configureAdditivity()
{
    Properties additivityProperties = properties.getPropertySubset("additivity.");
    vector<string> additivitysProps = additivityProperties.propertyNames();

    for(vector<string>::iterator it=additivitysProps.begin(); 
        it!=additivitysProps.end(); 
        ++it) 
    {
        Category cat = Category::getInstance(*it);
        string actualValue = additivityProperties.getProperty(*it);
        string value = tolower(actualValue);

        if(value == "true") {
            cat.setAdditivity(true);
        }
        else if(value == "false") {
            cat.setAdditivity(false);
        }
        else {
            getLogLog().warn("Invalid Additivity value: \"" + actualValue + "\"");
        }
    }

}


