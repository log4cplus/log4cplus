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

#include <cassert>
#include <log4cplus/helpers/socket.h>
#include <log4cplus/helpers/loglog.h>

#pragma comment(lib, "ws2_32.lib")


using namespace log4cplus;
using namespace log4cplus::helpers;


/////////////////////////////////////////////////////////////////////////////
// file LOCAL Classes
/////////////////////////////////////////////////////////////////////////////

namespace
{

enum WSInitStates
{
    WS_UNINITIALIZED,
    WS_INITIALIZING,
    WS_INITIALIZED
};


static WSADATA wsa;
static LONG volatile winsock_state = WS_UNINITIALIZED;


static
void
init_winsock ()
{
    // Quick check first to avoid the expensive interlocked compare
    // and exchange.
    if (winsock_state == WS_INITIALIZED)
        return;

    // Try to change the state to WS_INITIALIZING.
    LONG val = ::InterlockedCompareExchange (&winsock_state, WS_INITIALIZING,
        WS_UNINITIALIZED);
    switch (val)
    {
    case WS_UNINITIALIZED:
    {
        int ret = WSAStartup (MAKEWORD (2, 2), &wsa);
        if (ret != 0)
        {
            // Revert the state back to WS_UNINITIALIZED to unblock other
            // threads and let them throw exception.
            val = ::InterlockedCompareExchange (&winsock_state, WS_UNINITIALIZED,
                WS_INITIALIZING);
            assert (val == WS_INITIALIZING);
            throw std::runtime_error ("Could not initialize WinSock.");
        }

        // WinSock is initialized, change the state to WS_INITIALIZED.
        val = ::InterlockedCompareExchange (&winsock_state, WS_INITIALIZED,
            WS_INITIALIZING);
        assert (val == WS_INITIALIZING);
        return;
    }

    case WS_INITIALIZING:
        // Wait for state change.
        while (true)
        {
            switch (winsock_state)
            {
            case WS_INITIALIZED:
                return;

            case WS_INITIALIZING:
                ::Sleep (0);
                continue;
        
            default:
                assert (0);
                throw std::runtime_error ("Unknown WinSock state.");
            }
        }

    case WS_INITIALIZED:
        // WinSock is already initialized.
        return;

    default:
        assert (0);
        throw std::runtime_error ("Unknown WinSock state.");
    }
}


class WinSockInitializer
{
    ~WinSockInitializer ()
    {
        if (winsock_state == WS_INITIALIZED)
            WSACleanup ();
    }

static WinSockInitializer winSockInitializer;
};

WinSockInitializer WinSockInitializer::winSockInitializer;


} // namespace



/////////////////////////////////////////////////////////////////////////////
// Global Methods
/////////////////////////////////////////////////////////////////////////////

SOCKET_TYPE
log4cplus::helpers::openSocket(unsigned short port, SocketState& state)
{
    init_winsock ();

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
    init_winsock ();

    SOCKET sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if(sock == INVALID_SOCKET) {
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
log4cplus::helpers::acceptSocket(SOCKET_TYPE sock, SocketState& /*state*/)
{
    init_winsock ();

    return ::accept(sock, NULL, NULL);
}



int
log4cplus::helpers::closeSocket(SOCKET_TYPE sock)
{
    return ::closesocket(sock);
}



long
log4cplus::helpers::read(SOCKET_TYPE sock, SocketBuffer& buffer)
{
    long res, read = 0;
 
    do
    { 
        res = ::recv(sock, 
                     buffer.getBuffer() + read, 
                     static_cast<int>(buffer.getMaxSize() - read),
                     0);
        if( res <= 0 ) {
            return res;
        }
        read += res;
    } while( read < static_cast<long>(buffer.getMaxSize()) );
 
    return read;
}



long
log4cplus::helpers::write(SOCKET_TYPE sock, const SocketBuffer& buffer)
{
    return ::send(sock, buffer.getBuffer(), static_cast<int>(buffer.getSize()), 0);
}

