// Module:  Log4CPLUS
// File:    socketbuffer.h
// Created: 5/2003
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

/** @file */

#ifndef LOG4CPLUS_HELPERS_SOCKET_BUFFER_HEADER_
#define LOG4CPLUS_HELPERS_SOCKET_BUFFER_HEADER_

#include <log4cplus/config.hxx>
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

