// Module:  Log4CPLUS
// File:    propertyconfig.cxx
// Created: 3/2003
// Author:  Tad E. Smith
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//
// $Log: not supported by cvs2svn $

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
    configureLoggers();
    configureAdditivity();
}



//////////////////////////////////////////////////////////////////////////////
// log4cplus::PropertyConfigurator protected methods
//////////////////////////////////////////////////////////////////////////////

void
log4cplus::PropertyConfigurator::configureLoggers()
{
    if(properties.exists("rootLogger")) {
        Logger root = Logger::getRoot();
        configureLogger(root, properties.getProperty("rootLogger"));
    }

    Properties loggerProperties = properties.getPropertySubset("logger.");
    vector<string> loggers = loggerProperties.propertyNames();
    for(vector<string>::iterator it=loggers.begin(); it!=loggers.end(); ++it) {
        Logger log = Logger::getInstance(*it);
        configureLogger(log, loggerProperties.getProperty(*it));
    }
}



void
log4cplus::PropertyConfigurator::configureLogger(log4cplus::Logger logger, 
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
        getLogLog().error("PropertyConfigurator::configureLogger()- Invalid config " \
                          "string(Logger = " + logger.getName() + "): \"" + config + "\"");
        return;
    }

    // Set the loglevel
    string loglevel = tokens[0];
    if(loglevel != "INHERITED") {
        logger.setLogLevel( getLogLevelManager().fromString(loglevel) );
    }

    // Set the Appenders
    for(int j=1; j<tokens.size(); ++j) {
        AppenderMap::iterator appenderIt = appenders.find(tokens[j]);
        if(appenderIt == appenders.end()) {
            getLogLog().error("PropertyConfigurator::configureLogger()- Invalid " \
                              "appender: " + tokens[j]);
            continue;
        }
        logger.addAppender( (*appenderIt).second );
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
        Logger logger = Logger::getInstance(*it);
        string actualValue = additivityProperties.getProperty(*it);
        string value = tolower(actualValue);

        if(value == "true") {
            logger.setAdditivity(true);
        }
        else if(value == "false") {
            logger.setAdditivity(false);
        }
        else {
            getLogLog().warn("Invalid Additivity value: \"" + actualValue + "\"");
        }
    }

}


