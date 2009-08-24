// Module:  Log4CPLUS
// File:    socketappender.cxx
// Created: 5/2003
// Author:  Tad E. Smith
//
//
// Copyright 2003-2009 Tad E. Smith
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


namespace log4cplus {

#define LOG4CPLUS_MESSAGE_VERSION 2



//////////////////////////////////////////////////////////////////////////////
// SocketAppender ctors and dtor
//////////////////////////////////////////////////////////////////////////////

SocketAppender::SocketAppender(const tstring& host_,
    int port_, const tstring& serverName_)
: host(host_),
  port(port_),
  serverName(serverName_)
{
    openSocket();
}



SocketAppender::SocketAppender(const helpers::Properties & properties)
 : Appender(properties),
   port(9998)
{
    host = properties.getProperty( LOG4CPLUS_TEXT("host") );
    if(properties.exists( LOG4CPLUS_TEXT("port") )) {
        tstring const & tmp = properties.getProperty( LOG4CPLUS_TEXT("port") );
        port = atoi(LOG4CPLUS_TSTRING_TO_STRING(tmp).c_str());
    }
    serverName = properties.getProperty( LOG4CPLUS_TEXT("ServerName") );

    openSocket();
}



SocketAppender::~SocketAppender()
{
    destructorImpl();
}



//////////////////////////////////////////////////////////////////////////////
// SocketAppender public methods
//////////////////////////////////////////////////////////////////////////////

void 
SocketAppender::close()
{
    getLogLog().debug(LOG4CPLUS_TEXT("Entering SocketAppender::close()..."));
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
SocketAppender::append(const spi::InternalLoggingEvent& event)
{
    if(!socket.isOpen()) {
        openSocket();
        if(!socket.isOpen()) {
            getLogLog().error(LOG4CPLUS_TEXT("SocketAppender::append()- Cannot connect to server"));
            return;
        }
    }

    helpers::SocketBuffer buffer(LOG4CPLUS_MAX_MESSAGE_SIZE - sizeof(unsigned int));
    convertToBuffer (buffer, event, serverName);
    helpers::SocketBuffer msgBuffer(LOG4CPLUS_MAX_MESSAGE_SIZE);

    msgBuffer.appendInt(static_cast<unsigned>(buffer.getSize()));
    msgBuffer.appendBuffer(buffer);

    socket.write(msgBuffer);
}




/////////////////////////////////////////////////////////////////////////////
// namespace helpers methods
/////////////////////////////////////////////////////////////////////////////

namespace helpers
{


void
convertToBuffer(SocketBuffer & buffer,
    const spi::InternalLoggingEvent& event,
    const tstring& serverName)
{
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
}


spi::InternalLoggingEvent
readFromBuffer(SocketBuffer& buffer)
{
    unsigned char msgVersion = buffer.readByte();
    if(msgVersion != LOG4CPLUS_MESSAGE_VERSION) {
        LogLog * loglog = LogLog::getLogLog();
        loglog->warn(LOG4CPLUS_TEXT("readFromBuffer() received socket message with an invalid version"));
    }

    unsigned char sizeOfChar = buffer.readByte();

    tstring serverName = buffer.readString(sizeOfChar);
    tstring loggerName = buffer.readString(sizeOfChar);
    LogLevel ll = buffer.readInt();
    tstring ndc = buffer.readString(sizeOfChar);
    if(serverName.length() > 0) {
        if(ndc.length() == 0) {
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
