// Module:  Log4CPLUS
// File:    socketbuffer.cxx
// Created: 5/2003
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

#include <log4cplus/helpers/socketbuffer.h>
#include <log4cplus/helpers/loglog.h>

#ifndef WIN32
#  include <netdb.h>
#endif

using namespace log4cplus;
using namespace log4cplus::helpers;


//////////////////////////////////////////////////////////////////////////////
// SocketBuffer ctors and dtor
//////////////////////////////////////////////////////////////////////////////

log4cplus::helpers::SocketBuffer::SocketBuffer(size_t maxsize)
: maxsize(maxsize),
  size(0),
  pos(0),
  buffer(new char[maxsize])
{
}



log4cplus::helpers::SocketBuffer::SocketBuffer(const SocketBuffer& rhs)
{
    copy(rhs);
}



log4cplus::helpers::SocketBuffer::~SocketBuffer()
{
    delete buffer;
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
    ret = ntohs( *((unsigned short*)&buffer[pos]) );
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
    ret = ntohl( *((unsigned int*)&buffer[pos]) );
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

    *((unsigned short*)&buffer[pos]) = htons(val);
    pos += sizeof(unsigned short);
    size = pos;
}



void
log4cplus::helpers::SocketBuffer::appendInt(unsigned int val)
{
    if((pos + sizeof(unsigned int)) > maxsize) {
        getLogLog().error(LOG4CPLUS_TEXT("SocketBuffer::appendInt()- Attempt to write beyond end of buffer"));
        return;
    }

    *((unsigned int*)&buffer[pos]) = htonl(val);
    pos += sizeof(unsigned int);
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

    appendInt(strlen);
    memcpy(&buffer[pos], str.c_str(), strlen);
    pos += strlen;
    size = pos;
#else
    size_t strlen = str.length();

    if((pos + sizeof(unsigned int) + (strlen * 2)) > maxsize) {
        getLogLog().error(LOG4CPLUS_TEXT("SocketBuffer::appendString()- Attempt to write beyond end of buffer"));
        return;
    }

    appendInt(strlen);
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

    memcpy(&buffer[pos], buf.buffer, buf.getSize());
    pos += buf.getSize();
    size = pos;
}



