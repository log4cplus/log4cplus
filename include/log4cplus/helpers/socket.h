// Module:  Log4CPLUS
// File:    socket.h
// Created: 4/2003
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

#ifndef LOG4CPLUS_HELPERS_SOCKET_HEADER_
#define LOG4CPLUS_HELPERS_SOCKET_HEADER_

#include <log4cplus/config.h>
#include <log4cplus/tstring.h>
#include <log4cplus/helpers/socketbuffer.h>
#if defined(_WIN32)
#include <winsock.h>
#endif

namespace log4cplus {
    namespace helpers {

        enum SocketState { ok,
                           not_opened,
                           bad_address,
                           connection_failed,
                           broken_pipe, 
                           invalid_access_mode,
                           message_truncated
                         };

#if !defined(_WIN32)
        typedef int SOCKET_TYPE;
#define INVALID_SOCKET -1
#else
        typedef SOCKET SOCKET_TYPE;
#endif

        class LOG4CPLUS_EXPORT AbstractSocket {
        public:
          // ctor and dtor
            AbstractSocket();
            AbstractSocket(SOCKET_TYPE sock, SocketState state, int err);
            AbstractSocket(const AbstractSocket&);
            virtual ~AbstractSocket() = 0;

          // methods
            /// Close socket
            virtual void close();
            virtual bool isOpen() const;

            AbstractSocket& operator=(const AbstractSocket& rhs);

        protected:
          // Methods
            virtual void copy(const AbstractSocket& rhs);

          // Data
            SOCKET_TYPE sock;
            SocketState state;
            int err;
        };



        /**
         * This class implements client sockets (also called just "sockets").
         * A socket is an endpoint for communication between two machines.
         */
        class LOG4CPLUS_EXPORT Socket : public AbstractSocket {
        public:
          // ctor and dtor
            Socket();
            Socket(SOCKET_TYPE sock, SocketState state, int err);
            Socket(const tstring& address, int port);
            virtual ~Socket();

          // methods
            virtual bool read(SocketBuffer& buffer);
            virtual bool write(const SocketBuffer& buffer);
        };



        /**
         * This class implements server sockets. A server socket waits for
         * requests to come in over the network. It performs some operation
         * based on that request, and then possibly returns a result to the
         * requester.
         */
        class LOG4CPLUS_EXPORT ServerSocket : public AbstractSocket {
        public:
          // ctor and dtor
            ServerSocket(int port);
            virtual ~ServerSocket();

            Socket accept();
        };


        LOG4CPLUS_EXPORT SOCKET_TYPE openSocket(unsigned short port, SocketState& state);
        LOG4CPLUS_EXPORT SOCKET_TYPE connectSocket(const log4cplus::tstring& hostn,
                                                   unsigned short port, SocketState& state);
        LOG4CPLUS_EXPORT SOCKET_TYPE acceptSocket(SOCKET_TYPE sock, SocketState& state);
        LOG4CPLUS_EXPORT int closeSocket(SOCKET_TYPE sock);

        LOG4CPLUS_EXPORT size_t read(SOCKET_TYPE sock, SocketBuffer& buffer);
        LOG4CPLUS_EXPORT size_t write(SOCKET_TYPE sock, const SocketBuffer& buffer);

    } // end namespace helpers
} // end namespace log4cplus

#endif // LOG4CPLUS_HELPERS_SOCKET_HEADER_
