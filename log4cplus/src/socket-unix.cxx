// Module:  Log4CPLUS
// File:    socket-unix.cxx
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

#include <cstring>
#include <vector>
#include <algorithm>
#include <log4cplus/helpers/socket.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggingevent.h>

#if defined(__hpux__)
# ifndef _XOPEN_SOURCE_EXTENDED
# define _XOPEN_SOURCE_EXTENDED
# endif
# include <arpa/inet.h>
#endif
 

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__hpux__) || defined (__CYGWIN__)
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

    int optval = 1;
    setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval) );

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

    std::memcpy((char*)&server.sin_addr, hp->h_addr_list[0], hp->h_length);
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

    while(   (clientSock = ::accept(sock, (struct sockaddr*)&net_client, &len)) == -1
          && (errno == EINTR))
        ;

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



long
log4cplus::helpers::read(SOCKET_TYPE sock, SocketBuffer& buffer)
{
    long read = 0;
 
    do
    { 
        long res = ::read(sock, buffer.getBuffer() + read,
            buffer.getMaxSize() - read);
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
#if defined(MSG_NOSIGNAL)
    int flags = MSG_NOSIGNAL;
#else
    int flags = 0;
#endif
    return ::send( sock, buffer.getBuffer(), buffer.getSize(), flags );
}


tstring
log4cplus::helpers::getHostname (bool fqdn)
{
    char const * hostname = "unknown";
    int ret;
    std::vector<char> hn (1024, 0);

    while (true)
    {
        ret = ::gethostname (&hn[0], static_cast<int>(hn.size ()) - 1);
        if (ret == 0)
        {
            hostname = &hn[0];
            break;
        }
#if defined (LOG4CPLUS_HAVE_ENAMETOOLONG)
        else if (ret != 0 && errno == ENAMETOOLONG)
            // Out buffer was too short. Retry with buffer twice the size.
            hn.resize (hn.size () * 2, 0);
#endif
        else
            break;
    }

    if (ret != 0 || (ret == 0 && ! fqdn))
        return LOG4CPLUS_STRING_TO_TSTRING (hostname);

    struct ::hostent * hp = ::gethostbyname (hostname);
    if (hp)
        hostname = hp->h_name;

    return LOG4CPLUS_STRING_TO_TSTRING (hostname);
}

