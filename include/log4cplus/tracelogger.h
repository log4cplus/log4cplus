// -*- C++ -*-
// Module:  Log4CPLUS
// File:    tracelogger.h
// Created: 1/2009
// Author:  Vaclav Haisman
//
//
// Copyright 2009-2017 Tad E. Smith
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

/** @file */

#ifndef LOG4CPLUS_TRACELOGGER_H
#define LOG4CPLUS_TRACELOGGER_H

#include <log4cplus/config.hxx>

#if defined (LOG4CPLUS_HAVE_PRAGMA_ONCE)
#pragma once
#endif

#include <log4cplus/logger.h>
#if __has_include (<source_location>)
#  include <source_location>
#endif
#if __has_include (<experimental/source_location>)
#  include <experimental/source_location>
#endif


namespace log4cplus
{

namespace detail
{

#if defined (__cpp_lib_source_location) && __cpp_lib_source_location >= 201907L
using source_location = std::source_location;
#elif defined (__cpp_lib_experimental_source_location) && __cpp_lib_experimental_source_location >= 201505L
using source_location = std::experimental::source_location;
#else
#  error std::source_location is not available
#endif

} // namespace detail

/**
 * This class is used to produce "Trace" logging.  When an instance of
 * this class is created, it will log a <code>"ENTER: " + msg</code>
 * log message if TRACE_LOG_LEVEL is enabled for <code>logger</code>.
 * When an instance of this class is destroyed, it will log a
 * <code>"ENTER: " + msg</code> log message if TRACE_LOG_LEVEL is enabled
 * for <code>logger</code>.
 * <p>
 * @see LOG4CPLUS_TRACE
 */
class TraceLogger
{
public:
    TraceLogger(Logger l, log4cplus::tstring _msg,
        log4cplus::detail::source_location _location
            = log4cplus::detail::source_location::current ())
        : logger(std::move (l)), msg(std::move (_msg)), file(_location.file_name ()),
          function(_location.function_name ()), line(static_cast<int>(_location.line ()))
    {
        if(logger.isEnabledFor(TRACE_LOG_LEVEL))
            logger.forcedLog(TRACE_LOG_LEVEL, LOG4CPLUS_TEXT("ENTER: ") + msg,
                file, line, function);
    }

    ~TraceLogger()
    {
        if(logger.isEnabledFor(TRACE_LOG_LEVEL))
            logger.forcedLog(TRACE_LOG_LEVEL, LOG4CPLUS_TEXT("EXIT:  ") + msg,
                file, line, function);
    }

private:
    TraceLogger (TraceLogger const &) = delete;
    TraceLogger & operator = (TraceLogger const &) = delete;

    Logger logger;
    log4cplus::tstring msg;
    const char* file;
    const char* function;
    int line;
};


} // log4cplus


#endif // LOG4CPLUS_TRACELOGGER_H
