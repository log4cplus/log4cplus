// Module:  Log4CPLUS
// File:    clogger.cxx
// Created: 01/2011
// Author:  Jens Rehsack
//
//
// Copyright 2011 Jens Rehsack & Tad E. Smith
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <log4cplus/logger.h>
#include <log4cplus/clogger.h>
#include <log4cplus/appender.h>
#include <log4cplus/hierarchy.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggerimpl.h>
#include <log4cplus/configurator.h>
#include <log4cplus/streams.h>
#include <log4cplus/helpers/snprintf.h>

#include <cerrno>
#include <cstdio>
#include <cstdarg>
#include <cstring>

#include <sstream>

using namespace log4cplus;
using namespace log4cplus::helpers;

LOG4CPLUS_EXPORT int
log4cplus_file_configure(const log4cplus_char_t *pathname)
{
    if( !pathname )
	return EINVAL;

    try
    {
	PropertyConfigurator::doConfigure( pathname );
    }
    catch(std::exception &e)
    {
	return -1;
    }

    return 0;
}

LOG4CPLUS_EXPORT int
log4cplus_str_configure(const log4cplus_char_t *config)
{
    if( !config )
	return EINVAL;

    try
    {
	tstring s(config);
	tistringstream iss(s);
	PropertyConfigurator pc(iss);
	pc.configure();
    }
    catch(std::exception &e)
    {
	return -1;
    }

    return 0;
}

LOG4CPLUS_EXPORT int
log4cplus_basic_configure(void)
{
    try
    {
	BasicConfigurator::doConfigure();
    }
    catch(std::exception &e)
    {
	return -1;
    }

    return 0;
}

LOG4CPLUS_EXPORT void
log4cplus_shutdown(void)
{
    Logger::shutdown();
}

LOG4CPLUS_EXPORT bool
log4cplus_logger_exists(const log4cplus_char_t *name)
{
    bool retval = false;

    try
    {
        retval = Logger::exists(name);
    }
    catch(std::exception &e)
    {
    }

    return retval;
}

LOG4CPLUS_EXPORT bool
log4cplus_logger_is_enabled_for(const log4cplus_char_t *name, loglevel_t ll)
{
    bool retval = false;

    try
    {
	if( name )
	{
	    Logger logger = Logger::getInstance(name);
	    retval = logger.isEnabledFor(ll);
	}
	else
	{
	    Logger logger = Logger::getRoot();
	    retval = logger.isEnabledFor(ll);
	}
    }
    catch(std::exception &e)
    {
    }

    return retval;
}

LOG4CPLUS_EXPORT int
log4cplus_logger_log(const log4cplus_char_t *name, loglevel_t ll, const log4cplus_char_t *msgfmt, ...)
{
    int retval = -1;

    try
    {
	Logger logger;
	if( name )
	    logger = Logger::getInstance(name);
	else
	    logger = Logger::getRoot();

	if( logger.isEnabledFor(ll) )
	{
	    va_list ap;
	    va_start(ap, msgfmt);
            snprintf_buf buf;
	    const tchar * msg = buf.print(msgfmt, ap);
	    va_end(ap);
	    logger.log(ll, msg);
	}

	retval = 0;
    }
    catch(std::exception &e)
    {
    }

    return retval;
}

LOG4CPLUS_EXPORT int
log4cplus_logger_force_log(const log4cplus_char_t *name, loglevel_t ll, const log4cplus_char_t *msgfmt, ...)
{
    int retval = -1;

    try
    {
	Logger logger;
	if( name )
	    logger = Logger::getInstance(name);
	else
	    logger = Logger::getRoot();

	if( logger.isEnabledFor(ll) )
	{
	    va_list ap;
	    va_start(ap, msgfmt);
            snprintf_buf buf;
	    const tchar * msg = buf.print(msgfmt, ap);
	    va_end(ap);
	    logger.forcedLog(ll, msg);
	}

	retval = 0;
    }
    catch(std::exception &e)
    {
    }

    return retval;
}

