// Module:  Log4CPLUS
// File:    socketbuffer.cxx
// Created: 5/2003
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

#include <cstring>
#include <limits>
#include <log4cplus/helpers/socketbuffer.h>
#include <log4cplus/helpers/loglog.h>

#if !defined(_WIN32)
#  include <netdb.h>
#else
#include <winsock.h>
#endif

#if defined (__CYGWIN__) || defined (LOG4CPLUS_HAVE_NETINET_IN_H)
#include <netinet/in.h>
#endif

using namespace log4cplus;
using namespace log4cplus::helpers;


//////////////////////////////////////////////////////////////////////////////
// SocketBuffer ctors and dtor
//////////////////////////////////////////////////////////////////////////////

log4cplus::helpers::SocketBuffer::SocketBuffer(size_t maxsize_)
: maxsize(maxsize_),
  size(0),
  pos(0),
  buffer(new char[maxsize_])
{
}



log4cplus::helpers::SocketBuffer::SocketBuffer(const SocketBuffer& rhs)
    : log4cplus::helpers::LogLogUser ()
{
    copy(rhs);
}



log4cplus::helpers::SocketBuffer::~SocketBuffer()
{
    delete [] buffer;
}



SocketBuffer& 
log4cplus::helpers::SocketBuffer::operator=(const SocketBuffer& rhs)
{
    if(&rhs != this) {
        delete buffer;
        copy(rhs);
    }

    return *this;
}



void
log4cplus::helpers::SocketBuffer::copy(const SocketBuffer& r)
{
    SocketBuffer& rhs = const_cast<SocketBuffer&>(r);
    maxsize = rhs.maxsize;
    size = rhs.size;
    pos = rhs.pos;
    buffer = rhs.buffer;

    rhs.maxsize = 0;
    rhs.size = 0;
    rhs.pos = 0;
    rhs.buffer = 0;
}




//////////////////////////////////////////////////////////////////////////////
// SocketBuffer methods
//////////////////////////////////////////////////////////////////////////////

unsigned char
log4cplus::helpers::SocketBuffer::readByte()
{
    if(pos >= maxsize) {
        getLogLog().error(LOG4CPLUS_TEXT("SocketBuffer::readByte()- end of buffer reached"));
        return 0;
    }
    else if((pos + sizeof(unsigned char)) > maxsize) {
        getLogLog().error(LOG4CPLUS_TEXT("SocketBuffer::readByte()- Attempt to read beyond end of buffer"));
        return 0;
    }

    unsigned char ret = *((unsigned char*)&buffer[pos]);
    pos += sizeof(unsigned char);

    return ret;
}



unsigned short
log4cplus::helpers::SocketBuffer::readShort()
{
    if(pos >= maxsize) {
        getLogLog().error(LOG4CPLUS_TEXT("SocketBuffer::readShort()- end of buffer reached"));
        return 0;
    }
    else if((pos + sizeof(unsigned short)) > maxsize) {
        getLogLog().error(LOG4CPLUS_TEXT("SocketBuffer::readShort()- Attempt to read beyond end of buffer"));
        return 0;
    }

    unsigned short ret;
    std::memcpy(&ret, buffer + pos, sizeof(ret));
    ret = ntohs(ret);
    pos += sizeof(unsigned short);

    return ret;
}



unsigned int
log4cplus::helpers::SocketBuffer::readInt()
{
    if(pos >= maxsize) {
        getLogLog().error(LOG4CPLUS_TEXT("SocketBuffer::readInt()- end of buffer reached"));
        return 0;
    }
    else if((pos + sizeof(unsigned int)) > maxsize) {
        getLogLog().error(LOG4CPLUS_TEXT("SocketBuffer::readInt()- Attempt to read beyond end of buffer"));
        return 0;
    }

    unsigned int ret;
    std::memcpy (&ret, buffer + pos, sizeof(ret));
    ret = ntohl(ret);
    pos += sizeof(unsigned int);
    
    return ret;
}


tstring
log4cplus::helpers::SocketBuffer::readString(unsigned char sizeOfChar)
{
    size_t strlen = readInt();
    size_t bufferLen = strlen * sizeOfChar;

    if(strlen == 0) {
        return tstring();
    }
    if(pos > maxsize) {
        getLogLog().error(LOG4CPLUS_TEXT("SocketBuffer::readString()- end of buffer reached"));
        return tstring();
    }

    if((pos + bufferLen) > maxsize) {
        getLogLog().error(LOG4CPLUS_TEXT("SocketBuffer::readString()- Attempt to read beyond end of buffer"));
        bufferLen = (maxsize - 1) - pos;
        strlen = bufferLen / sizeOfChar;
    }

#ifndef UNICODE
    if(sizeOfChar == 1) {
        tstring ret(&buffer[pos], strlen);
        pos += strlen;
        return ret;
    }
    else if(sizeOfChar == 2) {
        tstring ret;
        for(tstring::size_type i=0; i<strlen; ++i) {
            unsigned short tmp = readShort();
            ret += static_cast<char>(tmp < 256 ? tmp : ' ');
        }
        return ret;
    }
    else {
        getLogLog().error(LOG4CPLUS_TEXT("SocketBuffer::readString()- Invalid sizeOfChar!!!!"));
    }

#else /* UNICODE */
    if(sizeOfChar == 1) {
        std::string ret(&buffer[pos], strlen);
        pos += strlen;
        return towstring(ret);
    }
    else if(sizeOfChar == 2) {
        tstring ret;
        for(tstring::size_type i=0; i<strlen; ++i) {
            ret += static_cast<tchar>(readShort());
        }
        return ret;
    }
    else {
        getLogLog().error(LOG4CPLUS_TEXT("SocketBuffer::readString()- Invalid sizeOfChar!!!!"));
    }
#endif

    return tstring();
}



void
log4cplus::helpers::SocketBuffer::appendByte(unsigned char val)
{
    if((pos + sizeof(unsigned char)) > maxsize) {
        getLogLog().error(LOG4CPLUS_TEXT("SocketBuffer::appendByte()- Attempt to write beyond end of buffer"));
        return;
    }

    *((unsigned char*)&buffer[pos]) = val;
    pos += sizeof(unsigned char);
    size = pos;
}



void
log4cplus::helpers::SocketBuffer::appendShort(unsigned short val)
{
    if((pos + sizeof(unsigned short)) > maxsize) {
        getLogLog().error(LOG4CPLUS_TEXT("SocketBuffer::appendShort()- Attempt to write beyond end of buffer"));
        return;
    }

    unsigned short s = htons(val);
    std::memcpy(buffer + pos, &s, sizeof (s));
    pos += sizeof(s);
    size = pos;
}



void
log4cplus::helpers::SocketBuffer::appendInt(unsigned int val)
{
    if((pos + sizeof(unsigned int)) > maxsize) {
        getLogLog().error(LOG4CPLUS_TEXT("SocketBuffer::appendInt()- Attempt to write beyond end of buffer"));
        return;
    }

    int i = htonl(val);
    std::memcpy(buffer + pos, &i, sizeof (i));
    pos += sizeof(i);
    size = pos;
}



void
log4cplus::helpers::SocketBuffer::appendSize_t(size_t val)
{
    if ((pos + sizeof(unsigned)) > maxsize)
    {
        getLogLog().error(LOG4CPLUS_TEXT("SocketBuffer::appendInt(size_t)- ")
            LOG4CPLUS_TEXT("Attempt to write beyond end of buffer"));
        return;
    }
    if (val > (std::numeric_limits<unsigned>::max) ())
    {
        getLogLog().error(LOG4CPLUS_TEXT("SocketBuffer::appendInt(size_t)-")
            LOG4CPLUS_TEXT(" Attempt to write value greater than")
            LOG4CPLUS_TEXT(" std::numeric_limits<unsigned>::max"));
        return;
    }

    unsigned st = htonl(static_cast<unsigned>(val));
    std::memcpy(buffer + pos, &st, sizeof(st));
    pos += sizeof(st);
    size = pos;
}


void
log4cplus::helpers::SocketBuffer::appendString(const tstring& str)
{
#ifndef UNICODE
    size_t strlen = str.length();

    if((pos + sizeof(unsigned int) + strlen) > maxsize) {
        getLogLog().error(LOG4CPLUS_TEXT("SocketBuffer::appendString()- Attempt to write beyond end of buffer"));
        return;
    }

    appendInt(static_cast<unsigned>(strlen));
    std::memcpy(&buffer[pos], str.data(), strlen);
    pos += strlen;
    size = pos;
#else
    size_t strlen = str.length();

    if((pos + sizeof(unsigned int) + (strlen * 2)) > maxsize) {
        getLogLog().error(LOG4CPLUS_TEXT("SocketBuffer::appendString()- Attempt to write beyond end of buffer"));
        return;
    }

    appendInt(static_cast<unsigned>(strlen));
    for(tstring::size_type i=0; i<str.length(); ++i) {
        appendShort(static_cast<unsigned short>(str[i]));
    }
#endif
}



void
log4cplus::helpers::SocketBuffer::appendBuffer(const SocketBuffer& buf)
{
    if((pos + buf.getSize()) > maxsize) {
        getLogLog().error(LOG4CPLUS_TEXT("SocketBuffer::appendBuffer()- Attempt to write beyond end of buffer"));
        return;
    }

    std::memcpy(&buffer[pos], buf.buffer, buf.getSize());
    pos += buf.getSize();
    size = pos;
}



