// Module:  Log4CPLUS
// File:    nullappender.cxx
// Created: 6/2003
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

#include <log4cplus/nullappender.h>

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;



///////////////////////////////////////////////////////////////////////////////
// log4cplus::NullAppender ctors and dtor
///////////////////////////////////////////////////////////////////////////////

log4cplus::NullAppender::NullAppender()
{
}


log4cplus::NullAppender::NullAppender(const log4cplus::helpers::Properties& properties)
: Appender(properties)
{
}



log4cplus::NullAppender::~NullAppender()
{
    destructorImpl();
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::NullAppender public methods
///////////////////////////////////////////////////////////////////////////////

void
log4cplus::NullAppender::close()
{
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::NullAppender protected methods
///////////////////////////////////////////////////////////////////////////////

// This method does not need to be locked since it is called by
// doAppend() which performs the locking
void
log4cplus::NullAppender::append(const spi::InternalLoggingEvent&)
{
}


