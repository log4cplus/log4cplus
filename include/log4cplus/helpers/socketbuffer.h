// Module:  Log4CPLUS
// File:    socketbuffer.h
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

/** @file */

#ifndef LOG4CPLUS_HELPERS_SOCKET_BUFFER_HEADER_
#define LOG4CPLUS_HELPERS_SOCKET_BUFFER_HEADER_

#include <log4cplus/config.h>
#include <log4cplus/tstring.h>
#include <log4cplus/helpers/logloguser.h>


namespace log4cplus {
    namespace helpers {

        /**
         *
         */
        class LOG4CPLUS_EXPORT SocketBuffer : protected log4cplus::helpers::LogLogUser
        {
        public:
            SocketBuffer(size_t max);
            SocketBuffer(const SocketBuffer& rhs);
            ~SocketBuffer();

            SocketBuffer& operator=(const SocketBuffer& rhs);

            char *getBuffer() const { return buffer; }
            size_t getMaxSize() const { return maxsize; }
            size_t getSize() const { return size; }
            void setSize(size_t s) { size = s; }
            size_t getPos() const { return pos; }

            unsigned char readByte();
            unsigned short readShort();
            unsigned int readInt();
            tstring readString(unsigned char sizeOfChar);

            void appendByte(unsigned char val);
            void appendShort(unsigned short val);
            void appendInt(unsigned int val);
            void appendSize_t(size_t val);
            void appendString(const tstring& str);
            void appendBuffer(const SocketBuffer& buffer);

        private:
          // Methods
            void copy(const SocketBuffer& rhs);

          // Data
            size_t maxsize;
            size_t size;
            size_t pos;
            char *buffer;
        };

    } // end namespace helpers
} // end namespace log4cplus

#endif // LOG4CPLUS_HELPERS_SOCKET_HEADER_

