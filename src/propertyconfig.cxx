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
    getLogLog().debug("PropertyConfigurator::configure()- Entering...");
    configureAppenders();
    configureCategories();
    getLogLog().debug("PropertyConfigurator::configure()- Exiting...");
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
    getLogLog().debug("PropertyConfigurator::configureCategory()- Entering...");
    getLogLog().debug("   Category = " + cat.getName());
    getLogLog().debug("   Config = " + config);

    string configString = config;

    // Remove all spaces from configString
    int pos = configString.find_first_of(' ');
    while(pos != string::npos) {
        configString.erase(pos, 1);
        pos = configString.find_first_of(' ');
    }
    getLogLog().debug("   Trimmed Config = " + configString);

    // "Tokenize" configString
    vector<string> configs( count(configString.begin(), configString.end(), ',')  + 1 );

    for(int i=0, count=0; i<configString.size(); ++i) {
        if(configString[i] == ',') {
            count++;
        }
        else {
            configs[count] += configString[i];
        }
    }

    if(configs[0].length() == 0) {
        getLogLog().error("PropertyConfigurator::configureCategory()- Invalid config " \
                          "string(Category = " + cat.getName() + "): \"" + config + "\"");
        return;
    }

    // Set the priority
    string priority = configs[0];
    getLogLog().debug("   Priority = " + priority);
    if(priority != "INHERITED") {
        cat.setPriority( static_cast<Priority::PriorityLevel>(Priority::toPriority(priority).toInt()) );
    }

    // Set the Appenders
    for(int j=1; j<configs.size(); ++j) {
        getLogLog().debug("   Appender = " + configs[j]);
        AppenderMap::iterator appenderIt = appenders.find(configs[j]);
        if(appenderIt == appenders.end()) {
            getLogLog().error("PropertyConfigurator::configureCategory()- Invalid " \
                              "appender: " + configs[j]);
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
            getLogLog().debug("PropertyConfigurator::configureAppenders()- Appender: " 
			      + *it);
            string factoryName = appenderProperties.getProperty(*it);
            getLogLog().debug("PropertyConfigurator::configureAppenders()- FactoryName: "
			      + factoryName);
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
		continue;
	    }

        }
    } // end for loop
}


