// Module:  Log4CPLUS
// File:    socket-unix.cxx
// Created: 4/2003
// Author:  Tad E. Smith
//
//
// Copyright 2003-2010 Tad E. Smith
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
#include <log4cplus/helpers/threads.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/helpers/syncprims.h>

#include <arpa/inet.h>
 
#ifdef LOG4CPLUS_HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef LOG4CPLUS_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef LOG4CPLUS_HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#include <errno.h>

#ifdef LOG4CPLUS_HAVE_NETDB_H
#include <netdb.h>
#endif

#include <unistd.h>

#include <cstring>
#include <vector>
#include <algorithm>


using namespace log4cplus;
using namespace log4cplus::helpers;


namespace
{

#if ! defined (LOG4CPLUS_SINGLE_THREADED)
// We need to use log4cplus::thread here to work around compilation
// problem on AIX.
static log4cplus::thread::Mutex ghbn_mutex;

#endif


static
int
get_host_by_name (char const * hostname, std::string * name,
    struct sockaddr_in * addr)
{
#if defined (LOG4CPLUS_HAVE_GETADDRINFO)
    struct addrinfo hints;
    std::memset (&hints, 0, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_CANONNAME;

    if (inet_addr (hostname) != static_cast<in_addr_t>(-1))
	hints.ai_flags |= AI_NUMERICHOST;

    struct addrinfo * res = 0;
    int ret = getaddrinfo (hostname, 0, &hints, &res);
    if (ret != 0)
        return ret;

    struct addrinfo const & ai = *res;
    assert (ai.ai_family == AF_INET);
    
    if (name)
        *name = ai.ai_canonname;

    if (addr)
        std::memcpy (addr, ai.ai_addr, ai.ai_addrlen);

    freeaddrinfo (res);

#else
#  if ! defined (LOG4CPLUS_SINGLE_THREADED)
    // We need to use log4cplus::thread here to work around
    // compilation problem on AIX.
    log4cplus::thread::MutexGuard guard (ghbn_mutex);
#  endif

    struct ::hostent * hp = gethostbyname (hostname);
    if (! hp)
        return 1;
    assert (hp->h_addrtype == AF_INET);

    if (name)
        *name = hp->h_name;

    if (addr)
    {
        assert (hp->h_length <= sizeof (addr->sin_addr));
        std::memcpy (&addr->sin_addr, hp->h_addr_list[0], hp->h_length);
    }

#endif

    return 0;
}


} // namespace


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
    SOCKET_TYPE sock;
    int retval;

    std::memset (&server, 0, sizeof (server));
    retval = get_host_by_name (LOG4CPLUS_TSTRING_TO_STRING(hostn).c_str(),
        0, &server);
    if (retval != 0)
        return INVALID_SOCKET;

    server.sin_port = htons(port);
    server.sin_family = AF_INET;

    sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        return INVALID_SOCKET;
    }

    while ((retval = ::connect(sock,
                reinterpret_cast<struct sockaddr *>(&server), sizeof (server))) == -1
          && (errno == EINTR))
        ;
    if(retval == INVALID_SOCKET) {
        ::close(sock);
        return INVALID_SOCKET;
    }

    state = ok;
    return sock;
}


namespace
{


// Some systems like HP-UX have socklen_t but accept() does not use it
// as type of its 3rd parameter. This wrapper works around this
// incompatibility.
template <typename sockaddr_ptr_type, typename socklen_type>
static
SOCKET_TYPE
accept_wrap (
    int (* accept_func) (int, sockaddr_ptr_type, socklen_type *),
    SOCKET_TYPE sock, struct sockaddr * sa, socklen_t * len)
{
    socklen_type l = *len;
    SOCKET_TYPE result = static_cast<SOCKET_TYPE>(accept_func (sock, sa, &l));
    *len = static_cast<socklen_t>(l);
    return result;
}


} // namespace


SOCKET_TYPE
log4cplus::helpers::acceptSocket(SOCKET_TYPE sock, SocketState& state)
{
    struct sockaddr_in net_client;
    socklen_t len = sizeof(struct sockaddr);
    SOCKET_TYPE clientSock;

    while(   (clientSock = accept_wrap (accept, sock,
                (struct sockaddr*)&net_client, &len)) == -1
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

    std::string full_hostname;
    ret = get_host_by_name (hostname, &full_hostname, 0);
    if (ret == 0)
        hostname = full_hostname.c_str ();

    return LOG4CPLUS_STRING_TO_TSTRING (hostname);
}

