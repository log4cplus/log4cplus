// Module:  Log4CPLUS
// File:    loglevel.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright 2001-2014 Tad E. Smith
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
#include <algorithm>


namespace log4cplus
{


namespace
{

// The strings here are not simple tstring constants to allow using these
// strings and log4cplus itself in client code during static variables
// initialization. If they are simple tstring constants then, due to undefined
// order of initialization between translation units, they might be
// uninitialized before they are used by the client code. One possible solution
// to this is to use compiler specific attributes and/or pragmas to influence
// initialization order/priority. Another solution is using function local
// static variables, which are initialized on first use. We choose this
// implementation because it is more portable than compiler specific means.

#define DEF_LL_STRING(_logLevel)                                        \
static tstring const & _logLevel ## _STRING ()                          \
{                                                                       \
    static tstring const str (LOG4CPLUS_TEXT (#_logLevel));             \
    return str;                                                         \
}

DEF_LL_STRING (OFF)
DEF_LL_STRING (FATAL)
DEF_LL_STRING (ERROR)
DEF_LL_STRING (WARN)
DEF_LL_STRING (INFO)
DEF_LL_STRING (DEBUG)
DEF_LL_STRING (TRACE)
DEF_LL_STRING (ALL)
DEF_LL_STRING (NOTSET)
DEF_LL_STRING (UNKNOWN)

#undef DEF_LL_STRING


static
tstring const &
defaultLogLevelToStringMethod(LogLevel ll)
{
    switch(ll) {
        case OFF_LOG_LEVEL:     return OFF_STRING();
        case FATAL_LOG_LEVEL:   return FATAL_STRING();
        case ERROR_LOG_LEVEL:   return ERROR_STRING();
        case WARN_LOG_LEVEL:    return WARN_STRING();
        case INFO_LOG_LEVEL:    return INFO_STRING();
        case DEBUG_LOG_LEVEL:   return DEBUG_STRING();
        case TRACE_LOG_LEVEL:   return TRACE_STRING();
        //case ALL_LOG_LEVEL:     return ALL_STRING();
        case NOT_SET_LOG_LEVEL: return NOTSET_STRING();
    };

    return internal::empty_str;
}


static
LogLevel
defaultStringToLogLevelMethod(const tstring& s)
{
    // Since C++11, accessing str[0] is always safe as it returns '\0' for
    // empty string.

    switch (s[0])
    {
#define DEF_LLMATCH(_chr, _logLevel)                 \
        case _chr: if (s == _logLevel ## _STRING ()) \
            return _logLevel ## _LOG_LEVEL; break;

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

} // namespace


void
initializeLogLevelStrings ()
{
    OFF_STRING();
    FATAL_STRING();
    ERROR_STRING();
    WARN_STRING();
    INFO_STRING();
    DEBUG_STRING();
    TRACE_STRING();
    ALL_STRING();
    NOTSET_STRING();
}


//////////////////////////////////////////////////////////////////////////////
// LogLevelManager ctors and dtor
//////////////////////////////////////////////////////////////////////////////

LogLevelManager::LogLevelManager()
{
    pushToStringMethod (defaultLogLevelToStringMethod);

    pushFromStringMethod (defaultStringToLogLevelMethod);
}


LogLevelManager::~LogLevelManager()
{ }



//////////////////////////////////////////////////////////////////////////////
// LogLevelManager public methods
//////////////////////////////////////////////////////////////////////////////

tstring const &
LogLevelManager::toString(LogLevel ll) const
{
    tstring const * ret;
    for (LogLevelToStringMethodRec const & rec : toStringMethods)
    {
        if (rec.use_1_0)
        {
            // Use TLS to store the result to allow us to return
            // a reference.
            tstring & ll_str = internal::get_ptd ()->ll_str;
            ll_str = rec.func_1_0 (ll);
            ret = &ll_str;
        }
        else
            ret = &rec.func (ll);

        if (! ret->empty ())
            return *ret;
    }

    return UNKNOWN_STRING();
}


LogLevel
LogLevelManager::fromString(const tstring& arg) const
{
    tstring const s = helpers::toUpper(arg);

    for (auto func : fromStringMethods)
    {
        LogLevel ret = func (s);
        if (ret != NOT_SET_LOG_LEVEL)
            return ret;
    }

    helpers::getLogLog ().error (
        LOG4CPLUS_TEXT ("Unrecognized log level: ")
        + arg);

    return NOT_SET_LOG_LEVEL;
}


void
LogLevelManager::pushToStringMethod(LogLevelToStringMethod newToString)
{
    toStringMethods.emplace (toStringMethods.begin (), newToString);
}


void
LogLevelManager::pushToStringMethod(LogLevelToStringMethod_1_0 newToString)
{
    toStringMethods.emplace (toStringMethods.begin(), newToString);
}


void
LogLevelManager::pushFromStringMethod(StringToLogLevelMethod newFromString)
{
    fromStringMethods.push_back (newFromString);
}


//
//
//

LogLevelManager::LogLevelToStringMethodRec::LogLevelToStringMethodRec ()
{ }


LogLevelManager::LogLevelToStringMethodRec::LogLevelToStringMethodRec (
    LogLevelToStringMethod f)
    : func {f}
    , use_1_0 {false}
{ }


LogLevelManager::LogLevelToStringMethodRec::LogLevelToStringMethodRec (
    LogLevelToStringMethod_1_0 f)
    : func_1_0 {f}
    , use_1_0 {true}
{ }

} // namespace log4cplus
