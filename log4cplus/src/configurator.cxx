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

#include <log4cplus/configurator.h>
#include <log4cplus/hierarchylocker.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/sleep.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/spi/factory.h>
#include <log4cplus/spi/loggerimpl.h>

#include <sys/stat.h>
#include <algorithm>
#include <vector>


namespace log4cplus
{

using namespace log4cplus::helpers;
using namespace log4cplus::spi;


void initializeLog4cplus();


namespace
{
    static tchar const DELIM_START[] = LOG4CPLUS_TEXT("${");
    static tchar const DELIM_STOP[] = LOG4CPLUS_TEXT("}");
    static size_t const DELIM_START_LEN = 2;
    static size_t const DELIM_STOP_LEN = 1;


    /**
     * Perform variable substitution in string <code>val</code> from
     * environment variables.
     *
     * <p>The variable substitution delimeters are <b>${</b> and <b>}</b>.
     *
     * <p>For example, if the System properties contains "key=value", then
     * the call
     * <pre>
     * string s;
     * substEnvironVars(s, "Value of key is ${key}.");
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
     * string s;
     * substEnvironVars(s, "Value of inexistentKey is [${inexistentKey}]");
     * </pre>
     * will set <code>s</code> to "Value of inexistentKey is []"
     *
     * @param val The string on which variable substitution is performed.
     * @param dest The result.
     */
    static
    bool
    substVars (tstring & dest, const tstring & val, Properties const & props,
        helpers::LogLog& loglog, unsigned flags)
    {
        tstring::size_type i = 0;
        tstring::size_type var_start, var_end;
        tstring pattern (val);
        tstring key;
        tstring replacement;
        bool changed = false;
        bool const empty_vars
            = !! (flags & PropertyConfigurator::fAllowEmptyVars);
        bool const shadow_env
            = !! (flags & PropertyConfigurator::fShadowEnvironment);
        bool const rec_exp
            = !! (flags & PropertyConfigurator::fRecursiveExpansion);

        while (true)
        {
            // Find opening paren of variable substitution.
            var_start = pattern.find(DELIM_START, i);
            if (var_start == tstring::npos)
            {
                dest = pattern;
                return changed;
            }

            // Find closing paren of variable substitution.
            var_end = pattern.find(DELIM_STOP, var_start);
            if (var_end == tstring::npos)
            {
                tostringstream buffer;
                buffer << '"' << pattern
                       << "\" has no closing brace. "
                       << "Opening brace at position " << var_start << ".";
                loglog.error(buffer.str());
                dest = val;
                return false;
            }
            
            key.assign (pattern, var_start + DELIM_START_LEN,
                var_end - (var_start + DELIM_START_LEN));
            replacement.clear ();
            if (shadow_env)
                replacement = props.getProperty (key);
            if (! shadow_env || ! empty_vars && replacement.empty ())
            {
                char const * env_var
                    = getenv(LOG4CPLUS_TSTRING_TO_STRING(key).c_str());
                if (env_var)
                    replacement = LOG4CPLUS_STRING_TO_TSTRING (env_var);
            }
            
            if (empty_vars || ! replacement.empty ())
            {
                // Substitute the variable with its value in place.
                pattern.replace (var_start, var_end + DELIM_STOP_LEN,
                    replacement);
                changed = true;
                if (rec_exp)
                    // Retry expansion on the same spot.
                    continue;
                else
                    // Move beyond the just substitued part.
                    i = var_start + replacement.size ();
            }
            else
                // Nothing has been subtituted, just move beyond the
                // unexpanded variable.
                i = var_end + DELIM_STOP_LEN;
        } // end while loop

    } // end substVars()

} // namespace



//////////////////////////////////////////////////////////////////////////////
// PropertyConfigurator ctor and dtor
//////////////////////////////////////////////////////////////////////////////

PropertyConfigurator::PropertyConfigurator(const tstring& propertyFile,
    Hierarchy& h, unsigned flags)
    : h(h)
    , propertyFilename(propertyFile)
    , properties(propertyFile)
    , flags (flags)
{
    init();
}


PropertyConfigurator::PropertyConfigurator(const helpers::Properties& props,
    Hierarchy& h, unsigned flags)
    : h(h)
    , propertyFilename( LOG4CPLUS_TEXT("UNAVAILABLE") )
    , properties( props )
    , flags (flags)
{
    init();
}


PropertyConfigurator::PropertyConfigurator(tistream& propertyStream,
    Hierarchy& h, unsigned flags)
    : h(h)
    , propertyFilename( LOG4CPLUS_TEXT("UNAVAILABLE") )
    , properties(propertyStream)
    , flags (flags)
{
    init();
}


void
PropertyConfigurator::init()
{
    replaceEnvironVariables();
    properties = properties.getPropertySubset( LOG4CPLUS_TEXT("log4cplus.") );
}


PropertyConfigurator::~PropertyConfigurator()
{
}



//////////////////////////////////////////////////////////////////////////////
// PropertyConfigurator static methods
//////////////////////////////////////////////////////////////////////////////

void
PropertyConfigurator::doConfigure(const tstring& file, Hierarchy& h,
    unsigned flags)
{
    PropertyConfigurator tmp(file, h, flags);
    tmp.configure();
}



//////////////////////////////////////////////////////////////////////////////
// PropertyConfigurator public methods
//////////////////////////////////////////////////////////////////////////////

void
PropertyConfigurator::configure()
{
    initializeLog4cplus();
    configureAppenders();
    configureLoggers();
    configureAdditivity();

    // Erase the appenders so that we are not artificially keeping them "alive".
    appenders.clear ();
}



//////////////////////////////////////////////////////////////////////////////
// PropertyConfigurator protected methods
//////////////////////////////////////////////////////////////////////////////

void
PropertyConfigurator::reconfigure()
{
    properties = Properties(propertyFilename);
    init();
    configure();
}


void
PropertyConfigurator::replaceEnvironVariables()
{
    std::vector<tstring> keys = properties.propertyNames();
    std::vector<tstring>::const_iterator it = keys.begin();
    tstring val, subKey, subVal;
    bool changed;
    do 
    {
        changed = false;
        for(; it != keys.end(); ++it)
        {
            tstring const & key = *it;
            val = properties.getProperty(key);
            subKey.clear ();
            if (substVars(subKey, key, properties, getLogLog(), flags))
            {
                properties.removeProperty(key);
                properties.setProperty(subKey, val);
                changed = true;
            }
            
            subVal.clear ();
            if (substVars(subVal, val, properties, getLogLog(), flags))
            {
                properties.setProperty(subKey, subVal);
                changed = true;
            }
        }
    }
    while (changed);
}



void
PropertyConfigurator::configureLoggers()
{
    if(properties.exists( LOG4CPLUS_TEXT("rootLogger") ))
    {
        Logger root = h.getRoot();
        configureLogger(root,
                        properties.getProperty(LOG4CPLUS_TEXT("rootLogger")));
    }

    Properties loggerProperties
        = properties.getPropertySubset(LOG4CPLUS_TEXT("logger."));
    std::vector<tstring> loggers = loggerProperties.propertyNames();
    for(std::vector<tstring>::iterator it=loggers.begin(); it!=loggers.end();
        ++it)
    {
        Logger log = getLogger(*it);
        configureLogger(log, loggerProperties.getProperty(*it));
    }
}



void
PropertyConfigurator::configureLogger(Logger logger, const tstring& config)
{
    // Remove all spaces from config
    tstring configString;
    remove_copy_if(config.begin(), config.end(),
        string_append_iterator<tstring>(configString),
        std::bind1st(std::equal_to<tchar>(), LOG4CPLUS_TEXT(' ')));

    // "Tokenize" configString
    std::vector<tstring> tokens;
    tokenize(configString, ',', std::back_inserter(tokens));

    if(tokens.size() == 0) {
        getLogLog().error(
            LOG4CPLUS_TEXT("PropertyConfigurator::configureLogger()")
            LOG4CPLUS_TEXT("- Invalid config string(Logger = ")
            + logger.getName()
            + LOG4CPLUS_TEXT("): \"")
            + config
            + LOG4CPLUS_TEXT("\""));
        return;
    }

    // Set the loglevel
    tstring loglevel = tokens[0];
    if (loglevel != LOG4CPLUS_TEXT("INHERITED"))
        logger.setLogLevel( getLogLevelManager().fromString(loglevel) );

    // Remove all existing appenders first so that we do not duplicate output.
    logger.removeAllAppenders ();

    // Set the Appenders
    for(std::vector<tstring>::size_type j=1; j<tokens.size(); ++j)
    {
        AppenderMap::iterator appenderIt = appenders.find(tokens[j]);
        if (appenderIt == appenders.end())
        {
            getLogLog().error(
                LOG4CPLUS_TEXT("PropertyConfigurator::configureLogger()")
                LOG4CPLUS_TEXT("- Invalid appender: ")
                + tokens[j]);
            continue;
        }
        addAppender(logger, appenderIt->second);
    }
}



void
PropertyConfigurator::configureAppenders()
{
    Properties appenderProperties =
        properties.getPropertySubset(LOG4CPLUS_TEXT("appender."));
    std::vector<tstring> appendersProps = appenderProperties.propertyNames();
    tstring factoryName;
    for(std::vector<tstring>::iterator it=appendersProps.begin();
        it != appendersProps.end(); ++it)
    {
        if( it->find( LOG4CPLUS_TEXT('.') ) == tstring::npos )
        {
            factoryName = appenderProperties.getProperty(*it);
            AppenderFactory* factory 
                = getAppenderFactoryRegistry().get(factoryName);
            if (factory == 0)
            {
                tstring err =
                    LOG4CPLUS_TEXT("PropertyConfigurator::configureAppenders()")
                    LOG4CPLUS_TEXT("- Cannot find AppenderFactory: ");
                getLogLog().error(err + factoryName);
                continue;
            }

            Properties properties
                = appenderProperties.getPropertySubset((*it) 
                    + LOG4CPLUS_TEXT("."));
            try
            {
                SharedAppenderPtr appender = factory->createObject(properties);
                if (appender.get() == 0)
                {
                    tstring err =
                        LOG4CPLUS_TEXT("PropertyConfigurator::")
                        LOG4CPLUS_TEXT("configureAppenders()")
                        LOG4CPLUS_TEXT("- Failed to create appender: ");
                    getLogLog().error(err + *it);
                }
                else
                {
                    appender->setName(*it);
                    appenders[*it] = appender;
                }
            }
            catch(std::exception& e)
            {
                tstring err =
                    LOG4CPLUS_TEXT("PropertyConfigurator::")
                    LOG4CPLUS_TEXT("configureAppenders()")
                    LOG4CPLUS_TEXT("- Error while creating Appender: ");
                getLogLog().error(err + LOG4CPLUS_C_STR_TO_TSTRING(e.what()));
            }
        }
    } // end for loop
}


void
PropertyConfigurator::configureAdditivity()
{
    Properties additivityProperties =
        properties.getPropertySubset(LOG4CPLUS_TEXT("additivity."));
    std::vector<tstring> additivitysProps = additivityProperties.propertyNames();

    tstring actualValue;
    tstring value;

    for(std::vector<tstring>::const_iterator it = additivitysProps.begin();
        it != additivitysProps.end(); ++it)
    {
        Logger logger = getLogger(*it);
        actualValue = additivityProperties.getProperty(*it);
        value = toLower(actualValue);

        if(value == LOG4CPLUS_TEXT("true"))
            logger.setAdditivity(true);
        else if(value == LOG4CPLUS_TEXT("false"))
            logger.setAdditivity(false);
        else
            getLogLog().warn(  LOG4CPLUS_TEXT("Invalid Additivity value: \"")
                             + actualValue
                             + LOG4CPLUS_TEXT("\""));
    }
}



Logger
PropertyConfigurator::getLogger(const tstring& name)
{
    return h.getInstance(name);
}


void
PropertyConfigurator::addAppender(Logger &logger, SharedAppenderPtr& appender)
{
    logger.addAppender(appender);
}



//////////////////////////////////////////////////////////////////////////////
// BasicConfigurator ctor and dtor
//////////////////////////////////////////////////////////////////////////////

BasicConfigurator::BasicConfigurator(Hierarchy& h)
    : PropertyConfigurator( LOG4CPLUS_TEXT(""), h )
{
    properties.setProperty(LOG4CPLUS_TEXT("rootLogger"),
                           LOG4CPLUS_TEXT("DEBUG, STDOUT"));
    properties.setProperty(LOG4CPLUS_TEXT("appender.STDOUT"),
                           LOG4CPLUS_TEXT("log4cplus::ConsoleAppender"));
}




BasicConfigurator::~BasicConfigurator()
{
}


//////////////////////////////////////////////////////////////////////////////
// BasicConfigurator static methods
//////////////////////////////////////////////////////////////////////////////

void
BasicConfigurator::doConfigure(Hierarchy& h)
{
    BasicConfigurator tmp(h);
    tmp.configure();
}


#if !defined(LOG4CPLUS_SINGLE_THREADED)

//////////////////////////////////////////////////////////////////////////////
// ConfigurationWatchDogThread implementation
//////////////////////////////////////////////////////////////////////////////

class ConfigurationWatchDogThread 
    : public thread::AbstractThread,
      public PropertyConfigurator
{
public:
    ConfigurationWatchDogThread(const tstring& file, unsigned int millis)
        : PropertyConfigurator(file),
          waitSecs(millis/1000),
          shouldTerminate(false),
          lastModTime(Time::gettimeofday()),
          lock(NULL)
    {
        updateLastModTime();
        if(waitSecs <= 0)
            waitSecs = 1;
    }
    
    void terminate() { shouldTerminate = true; }
    
protected:
    virtual void run();
    virtual Logger getLogger(const tstring& name);
    virtual void addAppender(Logger &logger, SharedAppenderPtr& appender);
    
    bool checkForFileModification();
    void updateLastModTime();
    virtual ~ConfigurationWatchDogThread(){}
    
private:
    unsigned int waitSecs;
    bool shouldTerminate;
    Time lastModTime;
    HierarchyLocker* lock;
};


void
ConfigurationWatchDogThread::run()
{
    while(!shouldTerminate)
    {
        helpers::sleep(waitSecs);
        bool modified = checkForFileModification();
        if(modified) {
            // Lock the Hierarchy
            HierarchyLocker theLock(h);
            lock = &theLock;

            // reconfigure the Hierarchy
            theLock.resetConfiguration();
            reconfigure();
            updateLastModTime();

            // release the lock
            lock = NULL;
        }

    }
}


Logger
ConfigurationWatchDogThread::getLogger(const tstring& name)
{
    if(lock)
        return lock->getInstance(name);
    else
        return PropertyConfigurator::getLogger(name);
}


void
ConfigurationWatchDogThread::addAppender(Logger& logger,
                                         SharedAppenderPtr& appender)
{
    if(lock)
        lock->addAppender(logger, appender);
    else
        PropertyConfigurator::addAppender(logger, appender);
}


bool
ConfigurationWatchDogThread::checkForFileModification()
{
    struct stat fileStatus;
    if(::stat(LOG4CPLUS_TSTRING_TO_STRING(propertyFilename).c_str(),
            &fileStatus) == -1)
        return false;  // stat() returned error, so the file must not exist
    Time modTime(fileStatus.st_mtime);
    bool modified = (modTime > lastModTime);

#if defined(HAVE_LSTAT)
    if(!modified && S_ISLNK(fileStatus.st_mode))
    {
        ::lstat(LOG4CPLUS_TSTRING_TO_STRING(propertyFilename).c_str(),
            &fileStatus);
        Time linkModTime(fileStatus.st_mtime);
        modified = (linkModTime > lastModTime);
    }
#endif

    return modified;
}



void
ConfigurationWatchDogThread::updateLastModTime()
{
    struct stat fileStatus;
    if(::stat(LOG4CPLUS_TSTRING_TO_STRING(propertyFilename).c_str(),
            &fileStatus) == -1)
        return;  // stat() returned error, so the file must not exist
    lastModTime = Time(fileStatus.st_mtime);
}



//////////////////////////////////////////////////////////////////////////////
// PropertyConfiguratorWatchDog ctor and dtor
//////////////////////////////////////////////////////////////////////////////

ConfigureAndWatchThread::ConfigureAndWatchThread(const tstring& file,
    unsigned int millis)
    : watchDogThread(0)
{
    watchDogThread = new ConfigurationWatchDogThread(file, millis);
    watchDogThread->configure();
    watchDogThread->start();
}


ConfigureAndWatchThread::~ConfigureAndWatchThread()
{
    if(watchDogThread.get() != 0)
        watchDogThread->terminate();
}


#endif


} // namespace log4cplus
