// Module:  Log4CPLUS
// File:    socket-win32.cxx
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

#include <log4cplus/helpers/socket.h>
#include <log4cplus/helpers/loglog.h>


using namespace log4cplus;
using namespace log4cplus::helpers;


/////////////////////////////////////////////////////////////////////////////
// file LOCAL Classes
/////////////////////////////////////////////////////////////////////////////

namespace {
    class WinSockInitializer {
    public:
        WinSockInitializer() {
            WSAStartup(MAKEWORD(1, 1), &wsa);
        }
        ~WinSockInitializer() {
            WSACleanup();
        }

        WSADATA wsa;
    } winSockInitializer;

}



/////////////////////////////////////////////////////////////////////////////
// Global Methods
/////////////////////////////////////////////////////////////////////////////

SOCKET_TYPE
log4cplus::helpers::openSocket(unsigned short port, SocketState& state)
{
    SOCKET sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if(sock == INVALID_SOCKET) {
        return sock;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    if(bind(sock, (struct sockaddr*)&server, sizeof(server)) != 0) {
        return INVALID_SOCKET;
    }

    if(::listen(sock, 10) != 0) {
        return INVALID_SOCKET;
    }

    state = ok;
    return sock;
}


SOCKET_TYPE
log4cplus::helpers::connectSocket(const log4cplus::tstring& hostn, 
                                  unsigned short port, SocketState& state)
{
    SOCKET sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        return INVALID_SOCKET;
    }

    unsigned long ip = INADDR_NONE;
    struct hostent *hp = ::gethostbyname( LOG4CPLUS_TSTRING_TO_STRING(hostn).c_str() );
    if(hp == 0 || hp->h_addrtype != AF_INET) {
        ip = inet_addr( LOG4CPLUS_TSTRING_TO_STRING(hostn).c_str() );
        if(ip == INADDR_NONE) {
            state = bad_address;
            return INVALID_SOCKET;
        }
    }

    struct sockaddr_in insock;
    insock.sin_port = htons(port);
    insock.sin_family = AF_INET;
    if(hp != 0) {
        memcpy(&insock.sin_addr, hp->h_addr, sizeof insock.sin_addr);
    }
    else {
        insock.sin_addr.S_un.S_addr = ip;
    }

    int retval;
    while(   (retval = ::connect(sock, (struct sockaddr*)&insock, sizeof(insock))) == -1
          && (WSAGetLastError() == WSAEINTR))
        ;
    if(retval == SOCKET_ERROR) {
        ::closesocket(sock);
        return INVALID_SOCKET;
    }

    int enabled = 1;
    if(setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&enabled, sizeof(enabled)) != 0) {
        ::closesocket(sock);    
        return INVALID_SOCKET;
    }

    state = ok;
    return sock;
}


SOCKET_TYPE
log4cplus::helpers::acceptSocket(SOCKET_TYPE sock, SocketState& state)
{
    return ::accept(sock, NULL, NULL);
}



int
log4cplus::helpers::closeSocket(SOCKET_TYPE sock)
{
    return ::closesocket(sock);
}



int
log4cplus::helpers::read(SOCKET_TYPE sock, SocketBuffer& buffer)
{
    return ::recv(sock, buffer.getBuffer(), buffer.getMaxSize(), 0);
}



int
log4cplus::helpers::write(SOCKET_TYPE sock, const SocketBuffer& buffer)
{
    return ::send(sock, buffer.getBuffer(), buffer.getSize(), 0);
}

