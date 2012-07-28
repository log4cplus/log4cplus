// Module:  Log4CPLUS
// File:    log4judpappender.cxx
// Created: 7/2012
// Author:  Siva Chandran P
//
//
// Copyright 2012 Siva Chandran P
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

#include <log4cplus/log4judpappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/streams.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/internal/internal.h>
#include <cstring>


namespace log4cplus
{

//////////////////////////////////////////////////////////////////////////////
// Log4jUdpAppender ctors and dtor
//////////////////////////////////////////////////////////////////////////////

Log4jUdpAppender::Log4jUdpAppender(const tstring& host_, int port_)
: host(host_),
  port(port_)
{
    openSocket();
}



Log4jUdpAppender::Log4jUdpAppender(const helpers::Properties & properties)
 : Appender(properties),
   port(5000)
{
    host = properties.getProperty( LOG4CPLUS_TEXT("host") );
    if(properties.exists( LOG4CPLUS_TEXT("port") )) {
        tstring tmp = properties.getProperty( LOG4CPLUS_TEXT("port") );
        port = std::atoi(LOG4CPLUS_TSTRING_TO_STRING(tmp).c_str());
    }

    openSocket();
}



Log4jUdpAppender::~Log4jUdpAppender()
{
    destructorImpl();
}



//////////////////////////////////////////////////////////////////////////////
// Log4jUdpAppender public methods
//////////////////////////////////////////////////////////////////////////////

void 
Log4jUdpAppender::close()
{
    helpers::getLogLog().debug(
        LOG4CPLUS_TEXT("Entering Log4jUdpAppender::close()..."));

    socket.close();
    closed = true;
}



//////////////////////////////////////////////////////////////////////////////
// Log4jUdpAppender protected methods
//////////////////////////////////////////////////////////////////////////////

void
Log4jUdpAppender::openSocket()
{
    if(!socket.isOpen()) {
        socket = helpers::Socket(host, port, true);
    }
}

void
Log4jUdpAppender::append(const spi::InternalLoggingEvent& event)
{
    if(!socket.isOpen()) {
        openSocket();
        if(!socket.isOpen()) {
            helpers::getLogLog().error(
                LOG4CPLUS_TEXT("Log4jUdpAppender::append()- Cannot connect to server"));
            return;
        }
    }

    internal::appender_sratch_pad & appender_sp
        = internal::get_appender_sp ();
    tostringstream & buffer = appender_sp.oss;
    detail::clear_tostringstream (buffer);

    buffer << LOG4CPLUS_TEXT("<log4j:event logger=\"")
           << event.getLoggerName()
           << LOG4CPLUS_TEXT("\" level=\"")
           // TODO: Some escaping of special characters is needed here.
           << getLogLevelManager().toString(event.getLogLevel())
           << LOG4CPLUS_TEXT("\" timestamp=\"")
           << event.getTimestamp().getFormattedTime(LOG4CPLUS_TEXT("%s%q"))
           << LOG4CPLUS_TEXT("\" thread=\"") << event.getThread()
           << LOG4CPLUS_TEXT("\">")

           << LOG4CPLUS_TEXT("<log4j:message>")
           // TODO: Some escaping of special characters is needed here.
           << event.getMessage()
           << LOG4CPLUS_TEXT("</log4j:message>")

           << LOG4CPLUS_TEXT("<log4j:NDC>")
           // TODO: Some escaping of special characters is needed here.
           << event.getNDC()
           << LOG4CPLUS_TEXT("</log4j:NDC>")

           << LOG4CPLUS_TEXT("<log4j:locationInfo file=\"")
           // TODO: Some escaping of special characters is needed here.
           << event.getFile()
           << LOG4CPLUS_TEXT("\" line=\"")
           << event.getLine()
           << LOG4CPLUS_TEXT("\"/>")
           << LOG4CPLUS_TEXT("</log4j:event>");
    
    // including null char
    std::size_t bufferSize = static_cast<std::streamoff>(buffer.tellp()) + 1;
    helpers::SocketBuffer socketBuffer(bufferSize);
    socketBuffer.setSize(bufferSize);
    std::memcpy(socketBuffer.getBuffer(), buffer.str().c_str(), bufferSize);

    bool ret = socket.write(socketBuffer);
    if(!ret)
    {
        helpers::getLogLog().error(
            LOG4CPLUS_TEXT(
                "Log4jUdpAppender::append()- Cannot write to server"));
    }
}

} // namespace log4cplus
