// Module:  Log4cplus
// File:    asyncappender.h
// Created: 1/2009
// Author:  Vaclav Haisman  
//
//
// Copyright (C) Vaclav Haisman. All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

/** @file */

#ifndef LOG4CPLUS_ASYNCAPPENDER_H
#define LOG4CPLUS_ASYNCAPPENDER_H

#include <log4cplus/config.hxx>
#include <log4cplus/helpers/queue.h>
#include <log4cplus/appender.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/helpers/threads.h>
#include <log4cplus/helpers/appenderattachableimpl.h>


namespace log4cplus
{


class LOG4CPLUS_EXPORT AsyncAppender
    : public Appender
    , public helpers::AppenderAttachableImpl
{
public:
    AsyncAppender (SharedAppenderPtr const & app, unsigned max_len);
    AsyncAppender (helpers::Properties const &);
    virtual ~AsyncAppender ();

    virtual void doAppend (spi::InternalLoggingEvent const &);
    virtual void close ();

protected:
    virtual void append (spi::InternalLoggingEvent const &);

    void init_queue_thread (unsigned);

    thread::AbstractThreadPtr queue_thread;
    thread::QueuePtr queue;

private:
    AsyncAppender (AsyncAppender const &);
    AsyncAppender & operator = (AsyncAppender const &);
};


typedef helpers::SharedObjectPtr<AsyncAppender> AsyncAppenderPtr;


} // namespace log4cplus


#endif // LOG4CPLUS_ASYNCAPPENDER_H
