// Module:  LOG4CPLUS
// File:    socketappender.h
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

/** @file */

#ifndef _LOG4CPLUS_SOCKET_APPENDER_HEADER_
#define _LOG4CPLUS_SOCKET_APPENDER_HEADER_

#include <log4cplus/config.hxx>
#include <log4cplus/appender.h>
#include <log4cplus/helpers/socket.h>

#ifndef UNICODE
#  define LOG4CPLUS_MAX_MESSAGE_SIZE (8*1024)
#else
#  define LOG4CPLUS_MAX_MESSAGE_SIZE (2*8*1024)
#endif


namespace log4cplus {

    /**
     * Sends {@link spi::InternalLoggingEvent} objects to a remote a log server.
     *
     * The SocketAppender has the following properties:
     *
     * <ul>
     *
     *   <li>Remote logging is non-intrusive as far as the log event
     *   is concerned. In other words, the event will be logged with
     *   the same time stamp, NDC, location info as if it were logged
     *   locally by the client.
     *
     *   <li>SocketAppenders do not use a layout.
     *
     *   <li>Remote logging uses the TCP protocol. Consequently, if
     *   the server is reachable, then log events will eventually arrive
     *   at the server.
     *
     *   <li>If the remote server is down, the logging requests are
     *   simply dropped. However, if and when the server comes back up,
     *   then event transmission is resumed transparently. This
     *   transparent reconneciton is performed by a <em>connector</em>
     *   thread which periodically attempts to connect to the server.
     *
     *   <li>Logging events are automatically <em>buffered</em> by the
     *   native TCP implementation. This means that if the link to server
     *   is slow but still faster than the rate of (log) event production
     *   by the client, the client will not be affected by the slow
     *   network connection. However, if the network connection is slower
     *   then the rate of event production, then the client can only
     *   progress at the network rate. In particular, if the network link
     *   to the the server is down, the client will be blocked.
     * 
     *   <li>On the other hand, if the network link is up, but the server
     *   is down, the client will not be blocked when making log requests
     *   but the log events will be lost due to server unavailability.
     * </ul>
     */
    class LOG4CPLUS_EXPORT SocketAppender : public Appender {
    public:
      // Ctors
        SocketAppender(const log4cplus::tstring& host, int port, 
                       const log4cplus::tstring& serverName = tstring());
        SocketAppender(const log4cplus::helpers::Properties properties);

      // Dtor
        ~SocketAppender();

      // Methods
        virtual void close();

    protected:
        void openSocket();
        virtual void append(const spi::InternalLoggingEvent& event);

      // Data
        log4cplus::helpers::Socket socket;
        log4cplus::tstring host;
        int port;
        log4cplus::tstring serverName;

    private:
      // Disallow copying of instances of this class
        SocketAppender(const SocketAppender&);
        SocketAppender& operator=(const SocketAppender&);
    };

    namespace helpers {
        LOG4CPLUS_EXPORT
        SocketBuffer convertToBuffer(const log4cplus::spi::InternalLoggingEvent& event,
                                     const log4cplus::tstring& serverName);

        LOG4CPLUS_EXPORT
        log4cplus::spi::InternalLoggingEvent readFromBuffer(SocketBuffer& buffer);
    } // end namespace helpers

} // end namespace log4cplus

#endif // _LOG4CPLUS_SOCKET_APPENDER_HEADER_

