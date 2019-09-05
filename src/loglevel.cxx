// Module:  Log4CPLUS
// File:    loglevel.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright 2001-2017 Tad E. Smith
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

#include <log4cplus/loglevel.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/internal/internal.h>
#include <log4cplus/internal/customloglevelmanager.h>
#include <algorithm>


namespace log4cplus
{


LogLevelTranslator::LogLevelTranslator () = default;

LogLevelTranslator::~LogLevelTranslator () = default;




namespace
{

static tstring const ALL_STRING (LOG4CPLUS_TEXT("ALL"));
static tstring const TRACE_STRING (LOG4CPLUS_TEXT("TRACE"));
static tstring const DEBUG_STRING (LOG4CPLUS_TEXT("DEBUG"));
static tstring const INFO_STRING (LOG4CPLUS_TEXT("INFO"));
static tstring const WARN_STRING (LOG4CPLUS_TEXT("WARN"));
static tstring const ERROR_STRING (LOG4CPLUS_TEXT("ERROR"));
static tstring const FATAL_STRING (LOG4CPLUS_TEXT("FATAL"));
static tstring const OFF_STRING (LOG4CPLUS_TEXT("OFF"));
static tstring const NOTSET_STRING (LOG4CPLUS_TEXT("NOTSET"));
static tstring const UNKNOWN_STRING (LOG4CPLUS_TEXT("UNKNOWN"));


class LOG4CPLUS_PRIVATE DefaultLogLevelTranslator
    : virtual public LogLevelTranslator
{
public:
    DefaultLogLevelTranslator () = default;

    ~DefaultLogLevelTranslator () override = default;

    log4cplus::tstring const &
    toString (LogLevel ll) const override
    {
        switch(ll) {
        case OFF_LOG_LEVEL:     return OFF_STRING;
        case FATAL_LOG_LEVEL:   return FATAL_STRING;
        case ERROR_LOG_LEVEL:   return ERROR_STRING;
        case WARN_LOG_LEVEL:    return WARN_STRING;
        case INFO_LOG_LEVEL:    return INFO_STRING;
        case DEBUG_LOG_LEVEL:   return DEBUG_STRING;
        case TRACE_LOG_LEVEL:   return TRACE_STRING;
        //case ALL_LOG_LEVEL:     return ALL_STRING;
        case NOT_SET_LOG_LEVEL: return NOTSET_STRING;
        }

        return internal::empty_str;
    }

    LogLevel
    fromString (const log4cplus::tstring_view& s) const override
    {
        // Since C++11, accessing str[0] is always safe as it returns '\0' for
        // empty string.

        switch (s[0])
        {
#define DEF_LLMATCH(_chr, _logLevel)                 \
        case LOG4CPLUS_TEXT (_chr):                  \
            if (s == _logLevel ## _STRING)           \
                return _logLevel ## _LOG_LEVEL;      \
            else                                     \
                break;

            DEF_LLMATCH ('O', OFF);
            DEF_LLMATCH ('F', FATAL);
            DEF_LLMATCH ('E', ERROR);
            DEF_LLMATCH ('W', WARN);
            DEF_LLMATCH ('I', INFO);
            DEF_LLMATCH ('D', DEBUG);
            DEF_LLMATCH ('T', TRACE);
            DEF_LLMATCH ('A', ALL);

#undef DEF_LLMATCH
        }

        return NOT_SET_LOG_LEVEL;
    }
};


class LOG4CPLUS_PRIVATE SingleLogLevelTranslator
    : virtual public LogLevelTranslator
{
public:
    SingleLogLevelTranslator (LogLevel ll, const log4cplus::tstring_view & name_)
        : log_level (ll)
        , name (name_)
    { }

    ~SingleLogLevelTranslator () override = default;

    log4cplus::tstring const &
    toString (LogLevel ll) const override
    {
        if (ll == log_level)
            return name;
        else
            return internal::empty_str;
    }

    LogLevel
    fromString (const log4cplus::tstring_view& s) const override
    {
        if (s == name)
            return log_level;
        else
            return NOT_SET_LOG_LEVEL;
    }

protected:
    LogLevel log_level;
    log4cplus::tstring name;
};

} // namespace



//////////////////////////////////////////////////////////////////////////////
// LogLevelManager ctors and dtor
//////////////////////////////////////////////////////////////////////////////

LogLevelManager::LogLevelManager()
{
    pushLogLevelTranslator (SharedLogLevelTranslatorPtr (new DefaultLogLevelTranslator ()));
}


LogLevelManager::~LogLevelManager() = default;



//////////////////////////////////////////////////////////////////////////////
// LogLevelManager public methods
//////////////////////////////////////////////////////////////////////////////

tstring const &
LogLevelManager::toString(LogLevel ll) const
{
#if ! defined (LOG4CPLUS_SINGLE_THREADED)
    std::shared_lock guard (mtx);
#endif

    for (auto & ptr : translator_list)
    {
        tstring const & ret = ptr->toString (ll);
        if (! ret.empty ())
            return ret;
    }

    return UNKNOWN_STRING;
}


LogLevel
LogLevelManager::fromString(const tstring_view& arg) const
{
    tstring const s = helpers::toUpper(arg);

#if ! defined (LOG4CPLUS_SINGLE_THREADED)
    std::shared_lock guard (mtx);
#endif

    for (auto & ptr : translator_list)
    {
        LogLevel ret = ptr->fromString (s);
        if (ret != NOT_SET_LOG_LEVEL)
            return ret;
    }

    helpers::getLogLog ().error (
        LOG4CPLUS_TEXT ("Unrecognized log level: ")
        + tstring (arg));

    return NOT_SET_LOG_LEVEL;
}


void
LogLevelManager::pushLogLevel(LogLevel ll, const log4cplus::tstring_view & name)
{
    if (ll < 0)
        helpers::getLogLog ().error (LOG4CPLUS_TEXT ("Log level value must be > 0"), true);

    if (name.empty ())
        helpers::getLogLog ().error (LOG4CPLUS_TEXT ("Log level name cannot be empty"), true);

    pushLogLevelTranslator (SharedLogLevelTranslatorPtr (new SingleLogLevelTranslator (ll, name)));
}


void
LogLevelManager::pushLogLevelTranslator(SharedLogLevelTranslatorPtr translator)
{
    if (! translator)
        helpers::getLogLog ().error (
            LOG4CPLUS_TEXT ("Log level translator object pointer must not be NULL"),
            true);

#if ! defined (LOG4CPLUS_SINGLE_THREADED)
    std::unique_lock guard (mtx);
#endif

    translator_list.push_back (std::move (translator));
}


} // namespace log4cplus
