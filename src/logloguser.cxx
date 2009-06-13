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

#include <log4cplus/helpers/logloguser.h>
#include <log4cplus/helpers/loglog.h>


namespace log4cplus { namespace helpers {


LogLogUser::LogLogUser()
{ }


LogLogUser::LogLogUser(const LogLogUser& rhs)
{ }


LogLogUser::~LogLogUser()
{ }


LogLog&
LogLogUser::getLogLog() const
{
    return *LogLog::getLogLog ();
}


LogLogUser&
LogLogUser::operator=(const LogLogUser& rhs)
{
    return *this;
}

} } // namespace log4cplus { namespace helpers {
