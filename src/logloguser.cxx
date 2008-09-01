// Module:  Log4CPLUS
// File:    logloguser.cxx
// Created: 6/2003
// Author:  Tad E. Smith
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

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

