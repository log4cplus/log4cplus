// Module:  LOG4CPLUS
// File:    configurator.cxx
// Created: 3/2003
// Author:  Tad E. Smith
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//
// $Log: not supported by cvs2svn $
// Revision 1.5  2003/04/19 21:35:14  tcsmith
// Replaced use of back_insert_iterator with string_append_iterator.
//
// Revision 1.4  2003/04/18 21:00:38  tcsmith
// Converted from std::string to log4cplus::tstring.
//
// Revision 1.3  2003/04/05 20:14:38  tcsmith
// Added replaceEnvironVariables() implementation.
//
// Revision 1.2  2003/04/04 00:31:14  tcsmith
// Changed to support the rename of propertyconfig.h to configurator.h
//
// Revision 1.1  2003/04/03 23:46:14  tcsmith
// Renamed from propertyconfig.cxx
//
// Revision 1.3  2003/04/03 01:21:36  tcsmith
// Changed to support the rename of Category to Logger and Priority to
// LogLevel.
//

#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/spi/factory.h>

#include <algorithm>

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::spi;

//////////////////////////////////////////////////////////////////////////////
// File LOCAL methods
//////////////////////////////////////////////////////////////////////////////

#define DELIM_START LOG4CPLUS_TEXT("${")
#define DELIM_STOP LOG4CPLUS_TEXT("}")
#define DELIM_START_LEN 2
#define DELIM_STOP_LEN 1

namespace {
    /**
     * Perform variable substitution in string <code>val</code> from
     * environment variables.
     *
     * <p>The variable substitution delimeters are <b>${</b> and <b>}</b>.
     *
     * <p>For example, if the System properties contains "key=value", then
     * the call
     * <pre>
     * string s = substEnvironVars("Value of key is ${key}.");
     * </pre>
     *
     * will set the variable <code>s</code> to "Value of key is value.".
     *
     * <p>If no value could be found for the specified key, then
     * substitution defaults to the empty string.
     *
     * <p>For example, if there is no environment variable "inexistentKey", 
     * then the call
     *
     * <pre>
     * string s = substEnvironVars("Value of inexistentKey is [${inexistentKey}]");
     * </pre>
     * will set <code>s</code> to "Value of inexistentKey is []"
     *
     * @param val The string on which variable substitution is performed.
     */
     log4cplus::tstring substEnvironVars(const log4cplus::tstring& val) {
     log4cplus::tstring sbuf;

        int i = 0;
        int j, k;

        while(true) {
            j=val.find(DELIM_START, i);
            if(j == log4cplus::tstring::npos) {
                if(i==0)
                    return val;
                else {
                    sbuf += val.substr(i);
                    return sbuf;
                }
            }
            else {
                sbuf += val.substr(i, j - i);
                k = val.find(DELIM_STOP, j);
                if(k == log4cplus::tstring::npos) {
                    getLogLog().error(   LOG4CPLUS_TEXT('"') + val
                                      +  LOG4CPLUS_TEXT("\" has no closing brace. ") \
                                         LOG4CPLUS_TEXT("Opening brace at position TODO."));
                    return val;
                }
                else {
                    j += DELIM_START_LEN;
                    log4cplus::tstring key = val.substr(j, k - j);
                    char* replacement = 
                            getenv(LOG4CPLUS_TSTRING_TO_STRING(key).c_str());

                    if(replacement != 0)
                        sbuf += LOG4CPLUS_STRING_TO_TSTRING(replacement);
                    i = k + DELIM_STOP_LEN;
                }
            }
        } // end while loop
    
    } // end substEnvironVars()
  
}



//////////////////////////////////////////////////////////////////////////////
// log4cplus::PropertyConfigurator ctor and dtor
//////////////////////////////////////////////////////////////////////////////

log4cplus::PropertyConfigurator::PropertyConfigurator(const log4cplus::tstring& propertyFile)
: propertyFilename(propertyFile),
  properties(propertyFile)
{
    replaceEnvironVariables();
    properties = properties.getPropertySubset( LOG4CPLUS_TEXT("log4cplus.") );
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
log4cplus::PropertyConfigurator::replaceEnvironVariables()
{

    std::vector<log4cplus::tstring> keys = properties.propertyNames();
    std::vector<log4cplus::tstring>::iterator it = keys.begin();
    for(; it!=keys.end(); ++it) {
        log4cplus::tstring key = *it;
        log4cplus::tstring val = properties.getProperty(key);
        log4cplus::tstring subKey = substEnvironVars(key);
        if(subKey != key) {
            properties.removeProperty(key);
            properties.setProperty(subKey, val);
        }

        log4cplus::tstring subVal = substEnvironVars(val);
        if(subVal != val) {
            properties.setProperty(subKey, subVal);
        }
    }

}



void
log4cplus::PropertyConfigurator::configureLoggers()
{
    if(properties.exists( LOG4CPLUS_TEXT("rootLogger") )) {
        Logger root = Logger::getRoot();
        configureLogger(root, 
                        properties.getProperty(LOG4CPLUS_TEXT("rootLogger")));
    }

    Properties loggerProperties = 
            properties.getPropertySubset(LOG4CPLUS_TEXT("logger."));
    vector<tstring> loggers = loggerProperties.propertyNames();
    for(vector<tstring>::iterator it=loggers.begin(); it!=loggers.end(); ++it) {
        Logger log = Logger::getInstance(*it);
        configureLogger(log, loggerProperties.getProperty(*it));
    }
}



void
log4cplus::PropertyConfigurator::configureLogger(log4cplus::Logger logger, 
                                                 const log4cplus::tstring& config)
{
    // Remove all spaces from config
    tstring configString;
    remove_copy_if(config.begin(), config.end(),
                   string_append_iterator<tstring>(configString),
                   bind1st(equal_to<tchar>(), ' '));

    // "Tokenize" configString
    vector<tstring> tokens;
    tokenize(configString, ',',
             back_insert_iterator<vector<tstring> >(tokens));

    if(tokens.size() == 0) {
        getLogLog().error(  LOG4CPLUS_TEXT("PropertyConfigurator::configureLogger()- Invalid config string(Logger = ")
                          + logger.getName() 
                          + LOG4CPLUS_TEXT("): \"") 
                          + config 
                          + LOG4CPLUS_TEXT("\""));
        return;
    }

    // Set the loglevel
    tstring loglevel = tokens[0];
    if(loglevel != LOG4CPLUS_TEXT("INHERITED")) {
        logger.setLogLevel( getLogLevelManager().fromString(loglevel) );
    }

    // Set the Appenders
    for(int j=1; j<tokens.size(); ++j) {
        AppenderMap::iterator appenderIt = appenders.find(tokens[j]);
        if(appenderIt == appenders.end()) {
        getLogLog().error(LOG4CPLUS_TEXT("PropertyConfigurator::configureLogger()- Invalid appender: ")
                          + tokens[j]);
            continue;
        }
        logger.addAppender( (*appenderIt).second );
    }
}



void
log4cplus::PropertyConfigurator::configureAppenders()
{
    Properties appenderProperties = 
         properties.getPropertySubset(LOG4CPLUS_TEXT("appender."));
    vector<tstring> appendersProps = appenderProperties.propertyNames();
    for(vector<tstring>::iterator it=appendersProps.begin(); 
        it!=appendersProps.end(); 
        ++it) 
    {
        if( (*it).find( LOG4CPLUS_TEXT('.') ) == tstring::npos ) {
            tstring factoryName = appenderProperties.getProperty(*it);
            AppenderFactory* factory = getAppenderFactoryRegistry().get(factoryName);
            if(factory == 0) {
                tstring err = 
                    LOG4CPLUS_TEXT("PropertyConfigurator::configureAppenders()- Cannot find AppenderFactory: ");
                getLogLog().error(err + factoryName);
                continue;
            }

            Properties properties = 
                    appenderProperties.getPropertySubset((*it) + LOG4CPLUS_TEXT("."));
            try {
                SharedAppenderPtr appender = factory->createObject(properties);
                if(appender.get() == 0) {
                    tstring err = 
                        LOG4CPLUS_TEXT("PropertyConfigurator::configureAppenders()- Failed to create appender: ");
                    getLogLog().error(err + *it);
                }
                else {
                    appender->setName(*it);
                    appenders[*it] = appender;
                }
            }
            catch(std::exception& e) {
                tstring err = 
                    LOG4CPLUS_TEXT("PropertyConfigurator::configureAppenders()- Error while creating Appender: ");
                getLogLog().error(err + LOG4CPLUS_C_STR_TO_TSTRING(e.what()));
            }
        }
    } // end for loop
}


void
log4cplus::PropertyConfigurator::configureAdditivity()
{
    Properties additivityProperties = 
            properties.getPropertySubset(LOG4CPLUS_TEXT("additivity."));
    vector<tstring> additivitysProps = additivityProperties.propertyNames();

    for(vector<tstring>::iterator it=additivitysProps.begin(); 
        it!=additivitysProps.end(); 
        ++it) 
    {
        Logger logger = Logger::getInstance(*it);
        tstring actualValue = additivityProperties.getProperty(*it);
        tstring value = tolower(actualValue);

        if(value == LOG4CPLUS_TEXT("true")) {
            logger.setAdditivity(true);
        }
        else if(value == LOG4CPLUS_TEXT("false")) {
            logger.setAdditivity(false);
        }
        else {
            getLogLog().warn(  LOG4CPLUS_TEXT("Invalid Additivity value: \"") 
                             + actualValue 
                             + LOG4CPLUS_TEXT("\""));
        }
    }

}



//////////////////////////////////////////////////////////////////////////////
// log4cplus::BasicConfigurator ctor and dtor
//////////////////////////////////////////////////////////////////////////////

log4cplus::BasicConfigurator::BasicConfigurator()
: log4cplus::PropertyConfigurator( LOG4CPLUS_TEXT("") )
{
    properties.setProperty(LOG4CPLUS_TEXT("rootLogger"), 
                           LOG4CPLUS_TEXT("DEBUG, STDOUT"));
    properties.setProperty(LOG4CPLUS_TEXT("appender.STDOUT"), 
                           LOG4CPLUS_TEXT("log4cplus::ConsoleAppender"));
}




log4cplus::BasicConfigurator::~BasicConfigurator()
{
}


