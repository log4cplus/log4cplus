// Module:  Log4CPLUS
// File:    loggingevent.cxx
// Created: 6/2003
// Author:  Tad E. Smith
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

#include <log4cplus/spi/loggingevent.h>
#include <algorithm>


namespace log4cplus {  namespace spi {


static const int LOG4CPLUS_DEFAULT_TYPE = 1;


///////////////////////////////////////////////////////////////////////////////
// InternalLoggingEvent ctors and dtor
///////////////////////////////////////////////////////////////////////////////

InternalLoggingEvent::InternalLoggingEvent(const log4cplus::tstring& _logger,
										   LogLevel _ll, 
										   const log4cplus::tstring& _message, 
										   const char* _filename,
										   int _line)
	: message(_message),
    loggerName(_logger),
    ll(_ll),
    timestamp(log4cplus::helpers::Time::gettimeofday()),
    line(_line),
    threadCached(false),
    ndcCached(false)
{
    if (_filename)
		file = LOG4CPLUS_C_STR_TO_TSTRING(_filename);

}


InternalLoggingEvent::InternalLoggingEvent(const log4cplus::tstring& _logger,
										   LogLevel _ll, 
										   const log4cplus::tstring& _ndc,
										   const log4cplus::tstring& _message, 
										   const log4cplus::tstring& _thread,
										   log4cplus::helpers::Time _time, 
										   const log4cplus::tstring& _file, 
										   int _line)
    : message(_message),
	  loggerName(_logger),
	  ll(_ll),
	  ndc(_ndc),
	  thread(_thread),
	  timestamp(_time),
	  file(_file),
	  line(_line),
	  threadCached(true),
	  ndcCached(true)
{
}


InternalLoggingEvent::InternalLoggingEvent ()
    : ll (NOT_SET_LOG_LEVEL),
	  threadCached(false),
	  ndcCached(false)
{ }


InternalLoggingEvent::InternalLoggingEvent(const log4cplus::spi::InternalLoggingEvent& rhs)
    : message(rhs.getMessage()),
	  loggerName(rhs.getLoggerName()),
	  ll(rhs.getLogLevel()),
	  ndc(rhs.getNDC()),
	  thread(rhs.getThread()),
	  timestamp(rhs.getTimestamp()),
	  file(rhs.getFile()),
	  line(rhs.getLine()),
	  threadCached(true),
	  ndcCached(true)
{
}


InternalLoggingEvent::~InternalLoggingEvent()
{
}



///////////////////////////////////////////////////////////////////////////////
// InternalLoggingEvent static methods
///////////////////////////////////////////////////////////////////////////////

unsigned int
InternalLoggingEvent::getDefaultType()
{
    return LOG4CPLUS_DEFAULT_TYPE;
}



///////////////////////////////////////////////////////////////////////////////
// InternalLoggingEvent implementation
///////////////////////////////////////////////////////////////////////////////

void
InternalLoggingEvent::setLoggingEvent (const log4cplus::tstring& _logger,
									   LogLevel _loglevel, 
									   const log4cplus::tstring& _msg, 
									   const char * _filename,
									   int _fline)
{
    loggerName = _logger;
    ll = _loglevel;
    message = _msg;
    timestamp = log4cplus::helpers::Time::gettimeofday();
    if (_filename)
        file = LOG4CPLUS_C_STR_TO_TSTRING (_filename);
    else
        file.clear();
    line = _fline;
    threadCached = false;
    ndcCached = false;
}


const log4cplus::tstring& 
InternalLoggingEvent::getMessage() const
{
    return message;
}


unsigned int
InternalLoggingEvent::getType() const
{
    return LOG4CPLUS_DEFAULT_TYPE;
}



std::auto_ptr<InternalLoggingEvent>
InternalLoggingEvent::clone() const
{
    std::auto_ptr<InternalLoggingEvent> tmp(new InternalLoggingEvent(*this));
    return tmp;
}



log4cplus::spi::InternalLoggingEvent&
InternalLoggingEvent::operator=(const log4cplus::spi::InternalLoggingEvent& rhs)
{
    if(this == &rhs) return *this;

    message = rhs.message;
    loggerName = rhs.loggerName;
    ll = rhs.ll;
    ndc = rhs.getNDC();
    thread = rhs.getThread();
    timestamp = rhs.timestamp;
    file = rhs.file;
    line = rhs.line;
    threadCached = true;
    ndcCached = true;

    return *this;
}


void
InternalLoggingEvent::swap (InternalLoggingEvent & other)
{
    std::swap(message, other.message);
    std::swap(loggerName, other.loggerName);
    std::swap(ll, other.ll);
    std::swap(ndc, other.ndc);
    std::swap(thread, other.thread);
    std::swap(timestamp, other.timestamp);
    std::swap(file, other.file);
    std::swap(line, other.line);
    std::swap(threadCached, other.threadCached);
    assert (threadCached);
    std::swap (ndcCached, other.ndcCached);
    assert (ndcCached);
}


} } // namespace log4cplus {  namespace spi {
