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
#include <log4cplus/helpers/source_location.h>
#include <utility>


namespace log4cplus
{


/**
 * This class is used to produce "Trace" logging.  When an instance of
 * this class is created, it will log a <code>"ENTER: " + msg</code>
 * log message if TRACE_LOG_LEVEL is enabled for <code>logger</code>.
 * When an instance of this class is destroyed, it will log a
 * <code>"EXIT:  " + msg</code> log message if TRACE_LOG_LEVEL is enabled
 * for <code>logger</code>.
 * The prefixes can be changed with setDefaultPrefixes(). Each instance
 * captures an immutable prefix pair at construction without copying its
 * strings, and uses that pair even if the defaults subsequently change.
 * <p>
 * @see LOG4CPLUS_TRACE_METHOD
 */
class TraceLogger
{
public:
    /** Complete prefixes, including any desired separators or whitespace. */
    struct Prefixes
    {
        tstring enterPrefix = LOG4CPLUS_TEXT("ENTER: ");
        tstring exitPrefix = LOG4CPLUS_TEXT("EXIT:  ");
    };

    /**
     * Returns the current immutable pair without copying its strings.
     * The reference remains valid through later configuration changes,
     * until the library context is destroyed. Normal library initialization
     * requirements apply.
     */
    static LOG4CPLUS_EXPORT Prefixes const & getDefaultPrefixes();

    /**
     * Publishes process-wide defaults for subsequent trace instances.
     * An empty prefix adds no text; Prefixes{} restores the built-in pair.
     * Changed pairs are retained until library-context teardown to keep
     * snapshot acquisition free of allocation, locking and reference counting.
     */
    static LOG4CPLUS_EXPORT void setDefaultPrefixes(Prefixes prefixes);

    TraceLogger(Logger l, log4cplus::tstring _msg,
        log4cplus::helpers::SourceLocation _location
            = log4cplus::helpers::SourceLocation::current ())
        : logger(std::move (l)), msg(std::move (_msg)), file(_location.file_name ()),
          function(_location.function_name ()), line(_location.line ()),
          prefixes(&getDefaultPrefixes())
    {
        if (logger.isEnabledFor(TRACE_LOG_LEVEL))
            logger.forcedLog(TRACE_LOG_LEVEL, prefixes->enterPrefix + msg,
                file, line, function);
    }

    TraceLogger(Logger l, log4cplus::tstring _msg,
        const char* _file, int _line, char const * _function)
        : logger(std::move (l)), msg(std::move (_msg)), file(_file),
          function(_function), line(_line), prefixes(&getDefaultPrefixes())
    {
        if (logger.isEnabledFor(TRACE_LOG_LEVEL))
            logger.forcedLog(TRACE_LOG_LEVEL, prefixes->enterPrefix + msg,
                file, line, function);
    }

    ~TraceLogger()
    {
        if (logger.isEnabledFor(TRACE_LOG_LEVEL))
            logger.forcedLog(TRACE_LOG_LEVEL, prefixes->exitPrefix + msg,
                file, line, function);
    }

    TraceLogger (TraceLogger const &) = delete;
    TraceLogger (TraceLogger &&) = delete;
    TraceLogger & operator = (TraceLogger const &) = delete;
    TraceLogger & operator = (TraceLogger &&) = delete;

private:
    Logger logger;
    log4cplus::tstring msg;
    const char* file;
    const char* function;
    int line;
    Prefixes const * prefixes;
};


} // log4cplus


#endif // LOG4CPLUS_TRACELOGGER_H
