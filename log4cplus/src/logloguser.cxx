// Module:  Log4CPLUS
// File:    logloguser.cxx
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

#include <log4cplus/streams.h>
#include <log4cplus/helpers/logloguser.h>
#include <log4cplus/helpers/loglog.h>

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;

typedef SharedObjectPtr<LogLog> LogLogPtr;



///////////////////////////////////////////////////////////////////////////////
// log4cplus::helpers::LogLogUser ctor and dtor
///////////////////////////////////////////////////////////////////////////////

LogLogUser::LogLogUser()
{
    loglogRef = new SharedObjectPtr<LogLog>(LogLog::getLogLog());
}


LogLogUser::LogLogUser(const LogLogUser& rhs)
{
    loglogRef = new SharedObjectPtr<LogLog>(*static_cast<LogLogPtr*>(rhs.loglogRef));
}


LogLogUser::~LogLogUser()
{
    delete static_cast<LogLogPtr*>(loglogRef);
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::helpers::LogLogUser public methods
///////////////////////////////////////////////////////////////////////////////

LogLog&
LogLogUser::getLogLog() const
{
    LogLogPtr* ptr = static_cast<LogLogPtr*>(loglogRef);
    return **ptr;
}


LogLogUser&
LogLogUser::operator=(const LogLogUser& rhs)
{
    if(this == &rhs) {
        return *this;
    }
    
    delete static_cast<LogLogPtr*>(loglogRef);
    loglogRef = new SharedObjectPtr<LogLog>(*static_cast<LogLogPtr*>(rhs.loglogRef));
    
    return *this;
}

