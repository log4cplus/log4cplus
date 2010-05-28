// Module:  Log4CPLUS
// File:    socketappender.cxx
// Created: 5/2003
// Author:  Tad E. Smith
//
//
// Copyright 2003-2010 Tad E. Smith
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

#include <cstdlib>
#include <log4cplus/socketappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/helpers/sleep.h>


int const LOG4CPLUS_MESSAGE_VERSION = 2;


namespace log4cplus
{

#if ! defined (LOG4CPLUS_SINGLE_THREADED)
SocketAppender::ConnectorThread::ConnectorThread (
    SocketAppender & socket_appender)
    : sa (socket_appender)
    , exit_flag (false)
{ }


SocketAppender::ConnectorThread::~ConnectorThread ()
{ }


void
SocketAppender::ConnectorThread::run ()
{
    while (true)
    {
        trigger_ev.timed_wait (30 * 1000);

        getLogLog().debug (
            LOG4CPLUS_TEXT("SocketAppender::ConnectorThread::run()")
            LOG4CPLUS_TEXT("- running..."));

        // Check exit condition as the very first thing.

        {
            thread::Guard guard (access_mutex);
            if (exit_flag)
                return;
            trigger_ev.reset ();
        }

        // Do not try to re-open already open socket.

        {
            thread::Guard guard (sa.access_mutex);
            if (sa.socket.isOpen ())
                continue;
        }

        // The socket is not open, try to reconnect.

        helpers::Socket socket (sa.host, sa.port);
        if (! socket.isOpen ())
        {
            getLogLog().error(
                LOG4CPLUS_TEXT("SocketAppender::ConnectorThread::run()")
                LOG4CPLUS_TEXT("- Cannot connect to server"));

            // Sleep for a short while after unsuccessful connection attempt
            // so that we do not try to reconnect after each logging attempt
            // which could be many times per second.
            helpers::sleep (5);

            continue;
        }

        // Connection was successful, move the socket into SocketAppender.

        {
            thread::Guard guard (sa.access_mutex);
            sa.socket = socket;
            sa.connected = true;
        }
    }
}


void
SocketAppender::ConnectorThread::terminate ()
{
    {
        thread::Guard guard (access_mutex);
        exit_flag = true;
        trigger_ev.signal ();
    }
    join ();
}


void
SocketAppender::ConnectorThread::trigger ()
{
    trigger_ev.signal ();
}

#endif


//////////////////////////////////////////////////////////////////////////////
// SocketAppender ctors and dtor
//////////////////////////////////////////////////////////////////////////////

SocketAppender::SocketAppender(const tstring& host_, int port_,
    const tstring& serverName_)
: host(host_),
  port(port_),
  serverName(serverName_)
{
    openSocket();
    initConnector ();
}



SocketAppender::SocketAppender(const helpers::Properties & properties)
 : Appender(properties),
   port(9998)
{
    host = properties.getProperty( LOG4CPLUS_TEXT("host") );
    if(properties.exists( LOG4CPLUS_TEXT("port") )) {
        tstring tmp = properties.getProperty( LOG4CPLUS_TEXT("port") );
        port = std::atoi(LOG4CPLUS_TSTRING_TO_STRING(tmp).c_str());
    }
    serverName = properties.getProperty( LOG4CPLUS_TEXT("ServerName") );

    openSocket();
    initConnector ();
}



SocketAppender::~SocketAppender()
{
#if ! defined (LOG4CPLUS_SINGLE_THREADED)
    connector->terminate ();
#endif

    destructorImpl();
}



//////////////////////////////////////////////////////////////////////////////
// SocketAppender public methods
//////////////////////////////////////////////////////////////////////////////

void 
SocketAppender::close()
{
    getLogLog().debug(LOG4CPLUS_TEXT("Entering SocketAppender::close()..."));

#if ! defined (LOG4CPLUS_SINGLE_THREADED)
    connector->terminate ();
#endif

    socket.close();
    closed = true;
}



//////////////////////////////////////////////////////////////////////////////
// SocketAppender protected methods
//////////////////////////////////////////////////////////////////////////////

void
SocketAppender::openSocket()
{
    if(!socket.isOpen()) {
        socket = helpers::Socket(host, port);
    }
}


void
SocketAppender::initConnector ()
{
#if ! defined (LOG4CPLUS_SINGLE_THREADED)
    connected = true;
    connector = new ConnectorThread (*this);
    connector->start ();
#endif
}


void
SocketAppender::append(const spi::InternalLoggingEvent& event)
{
#if ! defined (LOG4CPLUS_SINGLE_THREADED)
    if (! connected)
    {
        connector->trigger ();
        return;
    }

#else
    if(!socket.isOpen()) {
        openSocket();
        if(!socket.isOpen()) {
            getLogLog().error(LOG4CPLUS_TEXT("SocketAppender::append()- Cannot connect to server"));
            return;
        }
    }

#endif

    helpers::SocketBuffer buffer = helpers::convertToBuffer(event, serverName);
    helpers::SocketBuffer msgBuffer(LOG4CPLUS_MAX_MESSAGE_SIZE);

    msgBuffer.appendSize_t(buffer.getSize());
    msgBuffer.appendBuffer(buffer);

    bool ret = socket.write(msgBuffer);
    if (! ret)
    {
#if ! defined (LOG4CPLUS_SINGLE_THREADED)
        connected = false;
        connector->trigger ();
#endif
    }
}


/////////////////////////////////////////////////////////////////////////////
// namespace helpers methods
/////////////////////////////////////////////////////////////////////////////

namespace helpers
{

SocketBuffer
convertToBuffer(const spi::InternalLoggingEvent& event,
    const tstring& serverName)
{
    SocketBuffer buffer(LOG4CPLUS_MAX_MESSAGE_SIZE - sizeof(unsigned int));

    buffer.appendByte(LOG4CPLUS_MESSAGE_VERSION);
#ifndef UNICODE
    buffer.appendByte(1);
#else
    buffer.appendByte(2);
#endif

    buffer.appendString(serverName);
    buffer.appendString(event.getLoggerName());
    buffer.appendInt(event.getLogLevel());
    buffer.appendString(event.getNDC());
    buffer.appendString(event.getMessage());
    buffer.appendString(event.getThread());
    buffer.appendInt( static_cast<unsigned int>(event.getTimestamp().sec()) );
    buffer.appendInt( static_cast<unsigned int>(event.getTimestamp().usec()) );
    buffer.appendString(event.getFile());
    buffer.appendInt(event.getLine());

    return buffer;
}


spi::InternalLoggingEvent
readFromBuffer(SocketBuffer& buffer)
{
    unsigned char msgVersion = buffer.readByte();
    if(msgVersion != LOG4CPLUS_MESSAGE_VERSION) {
        SharedObjectPtr<LogLog> loglog = LogLog::getLogLog();
        loglog->warn(LOG4CPLUS_TEXT("readFromBuffer() received socket message with an invalid version"));
    }

    unsigned char sizeOfChar = buffer.readByte();

    tstring serverName = buffer.readString(sizeOfChar);
    tstring loggerName = buffer.readString(sizeOfChar);
    LogLevel ll = buffer.readInt();
    tstring ndc = buffer.readString(sizeOfChar);
    if(! serverName.empty ()) {
        if(ndc.empty ()) {
            ndc = serverName;
        }
        else {
            ndc = serverName + LOG4CPLUS_TEXT(" - ") + ndc;
        }
    }
    tstring message = buffer.readString(sizeOfChar);
    tstring thread = buffer.readString(sizeOfChar);
    long sec = buffer.readInt();
    long usec = buffer.readInt();
    tstring file = buffer.readString(sizeOfChar);
    int line = buffer.readInt();

    return spi::InternalLoggingEvent(loggerName,
                                                ll,
                                                ndc,
                                                message,
                                                thread,
                                                Time(sec, usec),
                                                file,
                                                line);
}

} // namespace helpers

} // namespace log4cplus
