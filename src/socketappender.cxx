// Module:  Log4CPLUS
// File:    socketappender.cxx
// Created: 5/2003
// Author:  Tad E. Smith
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//
// $Log: not supported by cvs2svn $
// Revision 1.7  2003/06/11 22:58:38  tcsmith
// Updated to support the changes in the InternalLoggingEvent class.
//
// Revision 1.6  2003/06/06 17:04:31  tcsmith
// Changed the ctor to take a 'const' Properties object.
//
// Revision 1.5  2003/06/03 20:22:00  tcsmith
// Modified the close() method to set "closed = true;".
//
// Revision 1.4  2003/05/21 22:14:46  tcsmith
// Fixed compiler warning: "conversion from 'size_t' to 'int', possible loss
// of data".
//
// Revision 1.3  2003/05/04 17:47:13  tcsmith
// Fixed for UNICODE.
//
// Revision 1.2  2003/05/04 08:40:30  tcsmith
// Fixed the readFromBuffer() to properly prepend the ServerName to the NDC.
//
// Revision 1.1  2003/05/04 07:25:16  tcsmith
// Initial version.
//

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

log4cplus::SocketAppender::SocketAppender(const log4cplus::tstring& host, int port,
                                          const log4cplus::tstring& serverName)
: host(host),
  port(port),
  serverName(serverName)
{
    openSocket();
}



log4cplus::SocketAppender::SocketAppender(const Properties properties)
 : Appender(properties),
   port(9998)
{
    host = properties.getProperty( LOG4CPLUS_TEXT("host") );
    if(properties.exists( LOG4CPLUS_TEXT("port") )) {
        tstring tmp = properties.getProperty( LOG4CPLUS_TEXT("port") );
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

    SocketBuffer buffer = convertToBuffer(event, serverName);
    SocketBuffer msgBuffer(LOG4CPLUS_MAX_MESSAGE_SIZE);

    msgBuffer.appendSize_t(buffer.getSize());
    msgBuffer.appendBuffer(buffer);

    socket.write(msgBuffer);
}




/////////////////////////////////////////////////////////////////////////////
// namespace log4cplus::helpers methods
/////////////////////////////////////////////////////////////////////////////

SocketBuffer
log4cplus::helpers::convertToBuffer(const log4cplus::spi::InternalLoggingEvent& event,
                                    const log4cplus::tstring& serverName)
{
    SocketBuffer buffer(LOG4CPLUS_MAX_MESSAGE_SIZE - sizeof(unsigned int));

    buffer.appendByte(LOG4CPLUS_MESSAGE_VERSION);
#ifndef UNICODE
    buffer.appendByte(1);
#else
    buffer.appendByte(2);
#endif

    buffer.appendString(serverName);
    buffer.appendString(event.loggerName);
    buffer.appendInt(event.ll);
    buffer.appendString(event.ndc);
    buffer.appendString(event.message);
    buffer.appendString(event.thread);
    buffer.appendInt( static_cast<unsigned int>(event.timestamp.sec()) );
    buffer.appendInt( static_cast<unsigned int>(event.timestamp.usec()) );
    buffer.appendString(event.file);
    buffer.appendInt(event.line);

    return buffer;
}


log4cplus::spi::InternalLoggingEvent
log4cplus::helpers::readFromBuffer(SocketBuffer& buffer)
{
    unsigned char msgVersion = buffer.readByte();
    if(msgVersion != LOG4CPLUS_MESSAGE_VERSION) {
        getLogLog().warn(LOG4CPLUS_TEXT("helpers::readFromBuffer() received socket message with an invalid version"));
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


