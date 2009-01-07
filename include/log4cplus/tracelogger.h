// Module:  Log4CPLUS
// File:    tracelogger.h
// Created: 1/2009
// Author:  Vaclav Haisman
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

/** @file */

#ifndef LOG4CPLUS_TRACELOGGER_H
#define LOG4CPLUS_TRACELOGGER_H

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>


namespace log4cplus
{


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
    TraceLogger(const Logger& l, const log4cplus::tstring& _msg,
                const char* _file=NULL, int _line=-1) 
      : logger(l), msg(_msg), file(_file), line(_line)
    { if(logger.isEnabledFor(TRACE_LOG_LEVEL))
          logger.forcedLog(TRACE_LOG_LEVEL, LOG4CPLUS_TEXT("ENTER: ") + msg, file, line); 
    }

    ~TraceLogger()
    { if(logger.isEnabledFor(TRACE_LOG_LEVEL))
          logger.forcedLog(TRACE_LOG_LEVEL, LOG4CPLUS_TEXT("EXIT:  ") + msg, file, line); 
    }

private:
    TraceLogger (TraceLogger const &);
    TraceLogger & operator = (TraceLogger const &);

    Logger logger;
    log4cplus::tstring msg;
    const char* file;
    int line;
};


} // log4cplus


#endif // LOG4CPLUS_TRACELOGGER_H
