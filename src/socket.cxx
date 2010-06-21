// Module:  Log4CPLUS
// File:    socket-win32.cxx
// Created: 4/2003
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

#include <log4cplus/helpers/socket.h>
#include <log4cplus/helpers/loglog.h>


using namespace log4cplus;
using namespace log4cplus::helpers;

#if !defined(_WIN32)
#  include <errno.h>
#  include <unistd.h>
#  define GET_LAST_ERROR errno
#else
#  define GET_LAST_ERROR WSAGetLastError()
#endif



//////////////////////////////////////////////////////////////////////////////
// AbstractSocket ctors and dtor
//////////////////////////////////////////////////////////////////////////////

log4cplus::helpers::AbstractSocket::AbstractSocket()
: sock(INVALID_SOCKET),
  state(not_opened),
  err(0)
{
}



log4cplus::helpers::AbstractSocket::AbstractSocket(SOCKET_TYPE sock_, SocketState state_, int err_)
: sock(sock_),
  state(state_),
  err(err_)
{
}



log4cplus::helpers::AbstractSocket::AbstractSocket(const log4cplus::helpers::AbstractSocket& rhs)
{
    copy(rhs);
}


log4cplus::helpers::AbstractSocket::~AbstractSocket()
{
    close();
}



//////////////////////////////////////////////////////////////////////////////
// AbstractSocket methods
//////////////////////////////////////////////////////////////////////////////

void
log4cplus::helpers::AbstractSocket::close()
{
    if(sock != INVALID_SOCKET) {
        closeSocket(sock);
        sock = INVALID_SOCKET;
    }
}



bool
log4cplus::helpers::AbstractSocket::isOpen() const
{
    return sock != INVALID_SOCKET;
}




log4cplus::helpers::AbstractSocket&
log4cplus::helpers::AbstractSocket::operator=(const log4cplus::helpers::AbstractSocket& rhs)
{
    if(&rhs != this) {
        close();
        copy(rhs);
    }

    return *this;
}



void
log4cplus::helpers::AbstractSocket::copy(const log4cplus::helpers::AbstractSocket& r)
{
    AbstractSocket& rhs = const_cast<AbstractSocket&>(r);
    sock = rhs.sock;
    state = rhs.state;
    err = rhs.err;
    rhs.sock = INVALID_SOCKET;
    rhs.state = not_opened;
    rhs.err = 0;
}



//////////////////////////////////////////////////////////////////////////////
// Socket ctors and dtor
//////////////////////////////////////////////////////////////////////////////

log4cplus::helpers::Socket::Socket()
: AbstractSocket()
{
}



log4cplus::helpers::Socket::Socket(const tstring& address, int port)
: AbstractSocket()
{
    sock = connectSocket(address, port, state);
    if(sock == INVALID_SOCKET) {
        err = GET_LAST_ERROR;
    }
}


log4cplus::helpers::Socket::Socket(SOCKET_TYPE sock_, SocketState state_, int err_)
: AbstractSocket(sock_, state_, err_)
{
}



log4cplus::helpers::Socket::~Socket()
{
}





//////////////////////////////////////////////////////////////////////////////
// Socket methods
//////////////////////////////////////////////////////////////////////////////

bool
log4cplus::helpers::Socket::read(SocketBuffer& buffer)
{
    long retval = log4cplus::helpers::read(sock, buffer);
    if(retval <= 0) {
        close();
    }
    else {
        buffer.setSize(retval);
    }

    return (retval > 0);
}



bool
log4cplus::helpers::Socket::write(const SocketBuffer& buffer)
{
    long retval = log4cplus::helpers::write(sock, buffer);
    if(retval <= 0) {
        close();
    }

    return (retval > 0);
}




//////////////////////////////////////////////////////////////////////////////
// ServerSocket ctor and dtor
//////////////////////////////////////////////////////////////////////////////

log4cplus::helpers::ServerSocket::ServerSocket(int port)
{
    sock = openSocket(port, state);
    if(sock == INVALID_SOCKET) {
        err = GET_LAST_ERROR;
    }
}



log4cplus::helpers::ServerSocket::~ServerSocket()
{
}



//////////////////////////////////////////////////////////////////////////////
// ServerSocket methods
//////////////////////////////////////////////////////////////////////////////

log4cplus::helpers::Socket
log4cplus::helpers::ServerSocket::accept()
{
    SocketState state_;
    SOCKET_TYPE clientSock = acceptSocket(sock, state_);
    return Socket(clientSock, state_, 0);
}


