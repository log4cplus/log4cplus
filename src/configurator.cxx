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
#include <set>
using namespace log4cplus::helpers;
using namespace log4cplus::spi;


//////////////////////////////////////////////////////////////////////////////
// Forward declarations
//////////////////////////////////////////////////////////////////////////////

namespace log4cplus {

void initializeLog4cplus();


//////////////////////////////////////////////////////////////////////////////
// File LOCAL methods
//////////////////////////////////////////////////////////////////////////////


namespace 
{
static tchar const DELIM_START[] = LOG4CPLUS_TEXT("${");
static tchar const DELIM_STOP[] = LOG4CPLUS_TEXT("}");
static size_t const DELIM_START_LEN = 2;
static size_t const DELIM_STOP_LEN = 1;


static
void
get_env_var (tstring & value, tstring const & name)
{
#if defined (WIN32) && defined (UNICODE)
	tchar const * val = _wgetenv (name.c_str ());
	if (val)
		value = val;
#else
	char const * val
		= std::getenv (LOG4CPLUS_TSTRING_TO_STRING (name).c_str ());
	if (val)
		value = LOG4CPLUS_STRING_TO_TSTRING (val);
#endif
}


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
substVars (tstring& dest, const tstring& val, 
		   const Properties& props,
		   helpers::LogLog& loglog, 
		   unsigned flags)
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
		if (! shadow_env || (! empty_vars && replacement.empty ()))
			get_env_var (replacement, key);
            
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
				// Move beyond the just substituted part.
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

PropertyConfigurator::PropertyConfigurator(const tstring& _propertyFile,
	Hierarchy& _h, 
	unsigned _flags)
	: h(_h),
	  propertyFilename(_propertyFile),
	  properties(_propertyFile),
	  flags (_flags)
{
init();
}


PropertyConfigurator::PropertyConfigurator(const helpers::Properties& _props,
	Hierarchy& _h, 
	unsigned _flags)
	: h(_h),
	  propertyFilename(LOG4CPLUS_TEXT("UNAVAILABLE")),
	  properties(_props),
	  flags(_flags)
{
	init();
}


PropertyConfigurator::PropertyConfigurator(tistream& _propertyStream,
										   Hierarchy& _h,
										   unsigned _flags)
	: h(_h),
	  propertyFilename(LOG4CPLUS_TEXT("UNAVAILABLE")),
	  properties(_propertyStream),
	  flags(_flags)
{
	init();
}


void
PropertyConfigurator::init()
{
	replaceEnvironVariables();
	properties = properties.getPropertySubset(LOG4CPLUS_TEXT("log4cplus."));
}


PropertyConfigurator::~PropertyConfigurator()
{
}



//////////////////////////////////////////////////////////////////////////////
// PropertyConfigurator static methods
//////////////////////////////////////////////////////////////////////////////

void
PropertyConfigurator::doConfigure(const tstring& file, 
								  Hierarchy& h,
								  unsigned flags)
{
	PropertyConfigurator(file, h, flags).configure();
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

	// Erase the appenders 
	// so that we are not artificially keeping them "alive".
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
	const std::set<tstring>& keys = properties.propertyNames();
	std::set<tstring>::const_iterator it = keys.begin();
	std::set<tstring>::const_iterator iter_end = keys.end();
	tstring val, subKey, subVal;
	bool changed;
	do 
	{
		changed = false;
		for(; it != iter_end; ++it)
		{
			val = properties.getProperty(*it);
			subKey.clear ();
			if (substVars(subKey, *it, properties, getLogLog(), flags))
			{
				properties.removeProperty(*it);
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
	const std::set<tstring>& loggers = loggerProperties.propertyNames();
	for(std::set<tstring>::const_iterator it=loggers.begin(); 
		it!=loggers.end();
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
				   std::bind1st(std::equal_to<tchar>(), 
								LOG4CPLUS_TEXT(' ')));

	// "Tokenize" configString
	std::vector<tstring> tokens;
	tokenize(configString, LOG4CPLUS_TEXT(','),
			 std::back_insert_iterator<std::vector<tstring> >(tokens));

	if (tokens.empty ())
	{
		tostringstream buffer;
		buffer << LOG4CPLUS_TEXT("PropertyConfigurator::configureLogger()")
			   << LOG4CPLUS_TEXT("- Invalid config string(Logger = ")
			   << logger.getName()
			   << LOG4CPLUS_TEXT("): \"")
			   << config
			   << LOG4CPLUS_TEXT("\"");

		getLogLog().error(buffer.str());
	}
	else
	{
		// Set the loglevel
		std::vector<tstring>::const_iterator iter = tokens.begin();
		const std::vector<tstring>::const_iterator iter_end = tokens.end();
		if (*iter != LOG4CPLUS_TEXT("INHERITED"))
			logger.setLogLevel(getLogLevelManager().fromString(*iter));
			
		// Remove all existing appenders first
		// so that we do not duplicate output.
		logger.removeAllAppenders();
			
		// Set the Appenders
		++iter;
		AppenderMap::iterator appenderIt;
		const AppenderMap::const_iterator appender_end = appenders.end();
		while (iter != iter_end)
		{
			if ( (appenderIt = appenders.find(*iter)) == appender_end)
			{
				tostringstream errorBuffer;
				errorBuffer  <<
					LOG4CPLUS_TEXT("PropertyConfigurator::configureLogger()")
					LOG4CPLUS_TEXT("Invalid appender: ") << *iter;
				getLogLog().error(errorBuffer.str());
			}
			else
				addAppender(logger, appenderIt->second);
			++iter;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

void
PropertyConfigurator::configureAppenders()
{
	typedef Properties::map_type data_type;
	typedef data_type::const_iterator const_iterator;

	Properties props = 
		properties.getPropertySubset(LOG4CPLUS_TEXT("appender."));
	const_iterator props_iter = props.data().begin();
	const const_iterator props_iter_end = props.data().end();

	for(; props_iter != props_iter_end; ++props_iter)
	{
		try 
		{
			if( props_iter->first.find( LOG4CPLUS_TEXT('.') ) == tstring::npos)
			{
				AppenderFactory* factory = 
					getAppenderFactoryRegistry().get(props_iter->second);
				if (!factory)
				{
					tostringstream errorBuffer;
					errorBuffer <<
						LOG4CPLUS_TEXT("PropertyConfigurator::")
						LOG4CPLUS_TEXT("configureAppenders()")
						LOG4CPLUS_TEXT("- Cannot find AppenderFactory: ") <<
						props_iter->second;
					getLogLog().error(errorBuffer.str());
				}
				else
				{
					
					Properties tmpProperties = 
						props.getPropertySubset(props_iter->first 
												+ LOG4CPLUS_TEXT("."));
					SharedAppenderPtr appender =
						factory->createObject(tmpProperties);
					if (!appender)
					{
						tostringstream errorBuffer;
						errorBuffer <<
							LOG4CPLUS_TEXT("PropertyConfigurator::")
							LOG4CPLUS_TEXT("configureAppenders()- ")
							LOG4CPLUS_TEXT("Failed to create appender: ")
							<< props_iter->first;
						getLogLog().error(errorBuffer.str());
					}
					else
					{
						appender->setName(props_iter->first);
						appenders.insert(std::make_pair(props_iter->first, appender));
					}
				}
			}
		}
		catch (const std::exception& e)
		{
			tostringstream errorBuffer;
			errorBuffer <<
				LOG4CPLUS_TEXT("PropertyConfigurator::")
				LOG4CPLUS_TEXT("configureAppenders()- ")
				LOG4CPLUS_TEXT("Error while creating Appender: ")  <<
				LOG4CPLUS_C_STR_TO_TSTRING(e.what());
			getLogLog().error(errorBuffer.str());
		}
	} // end for loop
}


void
PropertyConfigurator::configureAdditivity()
{
	const Properties additivityProperties =
		properties.getPropertySubset(LOG4CPLUS_TEXT("additivity."));

	const std::set<tstring>& additivitysProps = 
		additivityProperties.propertyNames();
	std::set<tstring>::const_iterator iter = additivitysProps.begin();
	const std::set<tstring>::const_iterator iter_end = additivitysProps.end();
	while (iter != iter_end)
	{
		Logger logger = getLogger(*iter);
		const tstring& actualValue = additivityProperties.getProperty(*iter);
		const tstring& lowerValue = toLower(actualValue);
			
		if(lowerValue == LOG4CPLUS_TEXT("true"))
			logger.setAdditivity(true);
		else if(lowerValue == LOG4CPLUS_TEXT("false"))
			logger.setAdditivity(false);
		else
		{
			tostringstream warnBuffer;
			warnBuffer << LOG4CPLUS_TEXT("Invalid Additivity value: \"")
					   << actualValue
					   << LOG4CPLUS_TEXT("\"");
			getLogLog().warn(warnBuffer.str());
		}
		++iter;
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

BasicConfigurator::BasicConfigurator(Hierarchy& _h)
	: PropertyConfigurator( LOG4CPLUS_TEXT(""), _h )
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

class ConfigurationWatchDogThread : public thread::AbstractThread,
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
	ConfigurationWatchDogThread(const ConfigurationWatchDogThread &);
	ConfigurationWatchDogThread& operator= (const ConfigurationWatchDogThread&);

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
	if (watchDogThread)
		watchDogThread->terminate();
}


#endif


} // namespace log4cplus
