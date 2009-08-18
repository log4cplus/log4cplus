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

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;

#define LOG4CPLUS_MESSAGE_VERSION 2



//////////////////////////////////////////////////////////////////////////////
// log4cplus::SocketAppender ctors and dtor
//////////////////////////////////////////////////////////////////////////////

log4cplus::SocketAppender::SocketAppender(const log4cplus::tstring& host_,
    int port_, const log4cplus::tstring& serverName_)
: host(host_),
  port(port_),
  serverName(serverName_)
{
    openSocket();
}



log4cplus::SocketAppender::SocketAppender(const Properties properties)
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



log4cplus::SocketAppender::~SocketAppender()
{
    destructorImpl();
}



//////////////////////////////////////////////////////////////////////////////
// log4cplus::SocketAppender public methods
//////////////////////////////////////////////////////////////////////////////

void 
log4cplus::SocketAppender::close()
{
    getLogLog().debug(LOG4CPLUS_TEXT("Entering SocketAppender::close()..."));
    socket.close();
    closed = true;
}



//////////////////////////////////////////////////////////////////////////////
// log4cplus::SocketAppender protected methods
//////////////////////////////////////////////////////////////////////////////

void
log4cplus::SocketAppender::openSocket()
{
    if(!socket.isOpen()) {
        socket = Socket(host, port);
    }
}



void
log4cplus::SocketAppender::append(const spi::InternalLoggingEvent& event)
{
    if(!socket.isOpen()) {
        openSocket();
        if(!socket.isOpen()) {
            getLogLog().error(LOG4CPLUS_TEXT("SocketAppender::append()- Cannot connect to server"));
            return;
        }
    }

    SocketBuffer buffer(LOG4CPLUS_MAX_MESSAGE_SIZE - sizeof(unsigned int));
    convertToBuffer (buffer, event, serverName);
    SocketBuffer msgBuffer(LOG4CPLUS_MAX_MESSAGE_SIZE);

    msgBuffer.appendInt(static_cast<unsigned>(buffer.getSize()));
    msgBuffer.appendBuffer(buffer);

    socket.write(msgBuffer);
}




/////////////////////////////////////////////////////////////////////////////
// namespace log4cplus::helpers methods
/////////////////////////////////////////////////////////////////////////////

void
log4cplus::helpers::convertToBuffer(SocketBuffer & buffer,
    const log4cplus::spi::InternalLoggingEvent& event,
    const log4cplus::tstring& serverName)
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


log4cplus::spi::InternalLoggingEvent
log4cplus::helpers::readFromBuffer(SocketBuffer& buffer)
{
    unsigned char msgVersion = buffer.readByte();
    if(msgVersion != LOG4CPLUS_MESSAGE_VERSION) {
        LogLog * loglog = log4cplus::helpers::LogLog::getLogLog();
        loglog->warn(LOG4CPLUS_TEXT("helpers::readFromBuffer() received socket message with an invalid version"));
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

    return log4cplus::spi::InternalLoggingEvent(loggerName,
                                                ll,
                                                ndc,
                                                message,
                                                thread,
                                                Time(sec, usec),
                                                file,
                                                line);
}


