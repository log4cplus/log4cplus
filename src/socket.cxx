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


#if !defined(_WIN32)
#  include <errno.h>
#  include <unistd.h>
#  define GET_LAST_ERROR errno
#else
#  define GET_LAST_ERROR WSAGetLastError()
#endif


namespace log4cplus { namespace helpers {


//////////////////////////////////////////////////////////////////////////////
// AbstractSocket ctors and dtor
//////////////////////////////////////////////////////////////////////////////

AbstractSocket::AbstractSocket()
: sock(INVALID_SOCKET),
  state(not_opened),
  err(0)
{
}



AbstractSocket::AbstractSocket(SOCKET_TYPE sock_,
    SocketState state_, int err_)
: sock(sock_),
  state(state_),
  err(err_)
{
}



AbstractSocket::AbstractSocket(const AbstractSocket& rhs)
{
    copy(rhs);
}


AbstractSocket::~AbstractSocket()
{
    close();
}



//////////////////////////////////////////////////////////////////////////////
// AbstractSocket methods
//////////////////////////////////////////////////////////////////////////////

void
AbstractSocket::close()
{
    if(sock != INVALID_SOCKET) {
        closeSocket(sock);
        sock = INVALID_SOCKET;
    }
}



bool
AbstractSocket::isOpen() const
{
    return sock != INVALID_SOCKET;
}




AbstractSocket&
AbstractSocket::operator=(const AbstractSocket& rhs)
{
    if(&rhs != this) {
        close();
        copy(rhs);
    }

    return *this;
}



void
AbstractSocket::copy(const AbstractSocket& r)
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

Socket::Socket()
: AbstractSocket()
{
}



Socket::Socket(const tstring& address, int port)
: AbstractSocket()
{
    sock = connectSocket(address, port, state);
    if(sock == INVALID_SOCKET) {
        err = errno;
    }
}


Socket::Socket(SOCKET_TYPE sock_, SocketState state_,
    int err_)
  : AbstractSocket(sock_, state_, err_)
{
}



Socket::~Socket()
{
}





//////////////////////////////////////////////////////////////////////////////
// Socket methods
//////////////////////////////////////////////////////////////////////////////

bool
Socket::read(SocketBuffer& buffer)
{
    long retval = helpers::read(sock, buffer);
    if(retval <= 0) {
        close();
    }
    else {
        buffer.setSize(retval);
    }

    return (retval > 0);
}



bool
Socket::write(const SocketBuffer& buffer)
{
    long retval = helpers::write(sock, buffer);
    if(retval <= 0) {
        close();
    }

    return (retval > 0);
}




//////////////////////////////////////////////////////////////////////////////
// ServerSocket ctor and dtor
//////////////////////////////////////////////////////////////////////////////

ServerSocket::ServerSocket(int port)
{
    sock = openSocket(port, state);
    if(sock == INVALID_SOCKET) {
        err = errno;
    }
}



ServerSocket::~ServerSocket()
{
}



//////////////////////////////////////////////////////////////////////////////
// ServerSocket methods
//////////////////////////////////////////////////////////////////////////////

Socket
ServerSocket::accept()
{
    SocketState st;
    SOCKET_TYPE clientSock = acceptSocket(sock, st);
    return Socket(clientSock, st, 0);
}


} } // namespace log4cplus { namespace helpers {
