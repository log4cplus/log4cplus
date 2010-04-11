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
#include <log4cplus/helpers/socket.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggingevent.h>

#if defined(__hpux__)
# ifndef _XOPEN_SOURCE_EXTENDED
# define _XOPEN_SOURCE_EXTENDED
# endif
# include <arpa/inet.h>
#endif 

#if defined (LOG4CPLUS_HAVE_NETINET_IN_H)
# include <netinet/in.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>


namespace log4cplus { namespace helpers {


/////////////////////////////////////////////////////////////////////////////
// Global Methods
/////////////////////////////////////////////////////////////////////////////

SOCKET_TYPE
openSocket(unsigned short port, SocketState& state)
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
    socklen_t optlen = sizeof (optval);
    setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &optval, optlen );

    int retval = bind(sock, reinterpret_cast<struct sockaddr*>(&server),
        sizeof(server));
    if (retval < 0)
        return INVALID_SOCKET;

    if (::listen(sock, 10))
        return INVALID_SOCKET;

    state = ok;
    return sock;
}


SOCKET_TYPE
connectSocket(const tstring& hostn, unsigned short port, SocketState& state)
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

    memcpy(&server.sin_addr, hp->h_addr_list[0], hp->h_length);
    server.sin_port = htons(port);
    server.sin_family = AF_INET;
    socklen_t namelen = sizeof (server);

    while (
        (retval = ::connect(sock, reinterpret_cast<struct sockaddr*>(&server),
            namelen))
        == -1
        && (errno == EINTR))
        ;
    if (retval == INVALID_SOCKET) 
    {
        ::close(sock);
        return INVALID_SOCKET;
    }

    state = ok;
    return sock;
}



SOCKET_TYPE
acceptSocket(SOCKET_TYPE sock, SocketState& state)
{
    struct sockaddr_in net_client;
    socklen_t len = sizeof(struct sockaddr);
    SOCKET_TYPE clientSock;

    while (
        (clientSock = ::accept(sock,
            reinterpret_cast<struct sockaddr*>(&net_client), &len)) 
        == -1
        && (errno == EINTR))
        ;

    if(clientSock != INVALID_SOCKET) {
        state = ok;
    }

    return clientSock;
}



int
closeSocket(SOCKET_TYPE sock)
{
    return ::close(sock);
}



long
read(SOCKET_TYPE sock, SocketBuffer& buffer)
{
    long res, readbytes = 0;
 
    do
    { 
        res = ::read(sock, buffer.getBuffer() + readbytes,
            buffer.getMaxSize() - readbytes);
        if( res <= 0 ) {
            return res;
        }
        readbytes += res;
    } while( readbytes < static_cast<long>(buffer.getMaxSize()) );
 
    return readbytes;
}



long
write(SOCKET_TYPE sock, const SocketBuffer& buffer)
{
#if defined(MSG_NOSIGNAL)
    int flags = MSG_NOSIGNAL;
#else
    int flags = 0;
#endif
    return ::send( sock, buffer.getBuffer(), buffer.getSize(), flags );
}


tstring
getHostname (bool fqdn)
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


} } // namespace log4cplus
