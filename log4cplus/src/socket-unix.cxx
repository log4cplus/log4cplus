// Module:  Log4CPLUS
// File:    socket-unix.cxx
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
// $Log: not supported by cvs2svn $
// Revision 1.4  2003/07/30 06:03:30  tcsmith
// Made changes to support Mac OS X builds.
//
// Revision 1.3  2003/07/19 15:30:22  tcsmith
// Changed the "errno.h" include statement for Solaris.
//
// Revision 1.2  2003/05/21 22:16:00  tcsmith
// Fixed compiler warning: "conversion from 'size_t' to 'int', possible loss
// of data".
//
// Revision 1.1  2003/05/04 07:25:16  tcsmith
// Initial version.
//

#include <log4cplus/helpers/socket.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggingevent.h>

#if defined(__APPLE__) || defined(__FreeBSD__)
#include <netinet/in.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>

#include <algorithm>

using namespace log4cplus;
using namespace log4cplus::helpers;


/////////////////////////////////////////////////////////////////////////////
// Global Methods
/////////////////////////////////////////////////////////////////////////////

SOCKET_TYPE
log4cplus::helpers::openSocket(unsigned short port, SocketState& state)
{
    SOCKET_TYPE sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        return INVALID_SOCKET;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if(bind(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        return INVALID_SOCKET;
    }

    if(::listen(sock, 10)) {
        return INVALID_SOCKET;
    }

    state = ok;
    return sock;
}


SOCKET_TYPE
log4cplus::helpers::connectSocket(const log4cplus::tstring& hostn,
                                  unsigned short port, SocketState& state)
{
    struct sockaddr_in server;
    struct hostent *hp;
    SOCKET_TYPE sock;
    int retval;

    hp = ::gethostbyname(LOG4CPLUS_TSTRING_TO_STRING(hostn).c_str());
    if(hp == 0) {
        return INVALID_SOCKET;
    }

    sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        return INVALID_SOCKET;
    }

    memcpy((char*)&server.sin_addr, hp->h_addr_list[0], hp->h_length);
    server.sin_port = htons(port);
    server.sin_family = AF_INET;

    while(   (retval = ::connect(sock, (struct sockaddr*)&server, sizeof(server))) == -1
          && (errno == EINTR))
        ;
    if(retval == INVALID_SOCKET) {
        ::close(sock);
        return INVALID_SOCKET;
    }

    state = ok;
    return sock;
}



SOCKET_TYPE
log4cplus::helpers::acceptSocket(SOCKET_TYPE sock, SocketState& state)
{
    struct sockaddr_in net_client;
    socklen_t len = sizeof(struct sockaddr);
    SOCKET_TYPE clientSock;
//    struct hostent *hostptr;

    while(   (clientSock = ::accept(sock, (struct sockaddr*)&net_client, &len)) == -1
          && (errno == EINTR))
        ;

//    hostptr = gethostbyaddr((char*)&(net_client.sin_addr.s_addr), 4, AF_INET);
    if(clientSock != INVALID_SOCKET) {
        state = ok;
    }

    return clientSock;
}



int
log4cplus::helpers::closeSocket(SOCKET_TYPE sock)
{
    return ::close(sock);
}



size_t
log4cplus::helpers::read(SOCKET_TYPE sock, SocketBuffer& buffer)
{
    return ::read(sock, buffer.getBuffer(), buffer.getMaxSize());
}



size_t
log4cplus::helpers::write(SOCKET_TYPE sock, const SocketBuffer& buffer)
{
    return ::write(sock, buffer.getBuffer(), buffer.getSize());
}



