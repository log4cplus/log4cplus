// Module:  Log4CPLUS
// File:    win32debugappender.cxx
// Created: 12/2003
// Author:  Eduardo Francos, Odalio SARL
//
//
// Copyright (C) Odalio SARL. All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//
// $Log: not supported by cvs2svn $
//

#include <log4cplus/win32debugappender.h>

#pragma comment(lib, "kernel32.lib")

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;



///////////////////////////////////////////////////////////////////////////////
// log4cplus::Win32DebugAppender ctors and dtor
///////////////////////////////////////////////////////////////////////////////

log4cplus::Win32DebugAppender::Win32DebugAppender()
{
}


log4cplus::Win32DebugAppender::Win32DebugAppender(
    const log4cplus::helpers::Properties& properties)
    : Appender(properties)
{
}



log4cplus::Win32DebugAppender::~Win32DebugAppender()
{
    destructorImpl();
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::Win32DebugAppender public methods
///////////////////////////////////////////////////////////////////////////////

void
log4cplus::Win32DebugAppender::close()
{
    closed = true;
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::Win32DebugAppender protected methods
///////////////////////////////////////////////////////////////////////////////

// This method does not need to be locked since it is called by
// doAppend() which performs the locking
void
log4cplus::Win32DebugAppender::append(const spi::InternalLoggingEvent& event)
{
    tostringstream buf;
    layout->formatAndAppend(buf, event);
    tstring sz = buf.str();
    const tchar * s = sz.c_str();
    ::OutputDebugString(s);
}
