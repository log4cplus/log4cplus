// Module:  LOG4CPLUS
// File:    configurator.cxx
// Created: 4/2003
// Author:  Michael CATANZARITI
//
// Copyright (C) Michael CATANZARITI  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//
// $Log: not supported by cvs2svn $
// Revision 1.2  2003/04/19 23:04:31  tcsmith
// Fixed UNICODE support.
//
// Revision 1.1  2003/04/19 21:31:07  tcsmith
// Initial version.
//

#include <log4cplus/nteventlogappender.h>
#include <log4cplus/loglevel.h>
#include <log4cplus/streams.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggingevent.h>


using namespace log4cplus;
using namespace log4cplus::spi;
using namespace log4cplus::helpers;


//////////////////////////////////////////////////////////////////////////////
// File LOCAL methods
//////////////////////////////////////////////////////////////////////////////

namespace {

    bool 
    FreeSid(SID* pSid) 
    {
        return ::HeapFree(GetProcessHeap(), 0, (LPVOID)pSid) != 0;
    }


    bool 
    CopySid(SID** ppDstSid, SID* pSrcSid) 
    {
        bool bSuccess = false;

        DWORD dwLength = ::GetLengthSid(pSrcSid);
        *ppDstSid = (SID *) ::HeapAlloc(GetProcessHeap(),
        HEAP_ZERO_MEMORY, dwLength);

        if(::CopySid(dwLength, *ppDstSid, pSrcSid)) {
            bSuccess = true;
        }
        else {
            FreeSid(*ppDstSid);
        }

        return bSuccess;
    }



    bool 
    GetCurrentUserSID(SID** ppSid) 
    {
        bool bSuccess = false;

        // Pseudohandle so don't need to close it
        HANDLE hProcess = ::GetCurrentProcess();
        HANDLE hToken = NULL;
        if(::OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
            // Get the required size
            DWORD tusize = 0;
            GetTokenInformation(hToken, TokenUser, NULL, 0, &tusize);
            TOKEN_USER* ptu = (TOKEN_USER*)new BYTE[tusize];

            if(GetTokenInformation(hToken, TokenUser, (LPVOID)ptu, tusize, &tusize)) {
                bSuccess = CopySid(ppSid, (SID *)ptu->User.Sid);
            }
            
            CloseHandle(hToken);
            delete [] ptu;
        }

        return bSuccess;
    }



    

    HKEY 
    regGetKey(const log4cplus::tstring& subkey, DWORD* disposition)
    {
        HKEY hkey = 0;
        RegCreateKeyEx(HKEY_LOCAL_MACHINE, 
                       subkey.c_str(), 
                       0, 
                       NULL, 
                       REG_OPTION_NON_VOLATILE, 
                       KEY_SET_VALUE, 
                       NULL, 
                       &hkey, 
                       disposition);
        return hkey;
    }



    void 
    regSetString(HKEY hkey, const log4cplus::tstring& name, const log4cplus::tstring& value)
    {
        RegSetValueEx(hkey, 
                      name.c_str(), 
                      0, 
                      REG_SZ, 
                      (LPBYTE)value.c_str(), 
                      static_cast<DWORD>(value.length() * sizeof(tchar)));
    }



    void 
    regSetDword(HKEY hkey, const log4cplus::tstring& name, DWORD value)
    {
        RegSetValueEx(hkey, 
                      name.c_str(), 
                      0, 
                      REG_DWORD, 
                      (LPBYTE)&value, 
                      sizeof(DWORD));
    }

}



//////////////////////////////////////////////////////////////////////////////
// log4cplus::NTEventLogAppender ctor and dtor
//////////////////////////////////////////////////////////////////////////////

NTEventLogAppender::NTEventLogAppender(const log4cplus::tstring& server, 
                                       const log4cplus::tstring& log, 
                                       const log4cplus::tstring& source)
: server(server), 
  log(log), 
  source(source), 
  hEventLog(NULL), 
  pCurrentUserSID(NULL)
{
    init();
}



NTEventLogAppender::NTEventLogAppender(log4cplus::helpers::Properties properties)
: Appender(properties),
  hEventLog(NULL), 
  pCurrentUserSID(NULL)
{
    server = properties.getProperty( LOG4CPLUS_TEXT("server") );
    log = properties.getProperty( LOG4CPLUS_TEXT("log") );
    source = properties.getProperty( LOG4CPLUS_TEXT("source") );

    init();
}



void 
NTEventLogAppender::init()
{
    if(source.empty()) {
        getLogLog().warn(  LOG4CPLUS_TEXT("Source option not set for appender [")
                         + name 
                         + LOG4CPLUS_TEXT("]."));
        return;
    }

    if(log.length() == 0) {
        log = LOG4CPLUS_TEXT("Application");
    }

    // current user security identifier
    GetCurrentUserSID(&pCurrentUserSID);

    addRegistryInfo();

    hEventLog = ::RegisterEventSource(server.c_str(), source.c_str());
}



NTEventLogAppender::~NTEventLogAppender()
{
    destructorImpl();
}



//////////////////////////////////////////////////////////////////////////////
// log4cplus::NTEventLogAppender public methods
//////////////////////////////////////////////////////////////////////////////

void 
NTEventLogAppender::close()
{
    if(hEventLog != NULL) {
        ::DeregisterEventSource(hEventLog);
        hEventLog = NULL;
    }

    if(pCurrentUserSID != NULL) {
        FreeSid(pCurrentUserSID);
        pCurrentUserSID = NULL;
    }
}



//////////////////////////////////////////////////////////////////////////////
// log4cplus::NTEventLogAppender protected methods
//////////////////////////////////////////////////////////////////////////////

void 
NTEventLogAppender::append(const InternalLoggingEvent& event)
{
    if(hEventLog == NULL) {
        getLogLog().warn(LOG4CPLUS_TEXT("NT EventLog not opened."));
        return;
    }

    tostringstream buf;
    layout->formatAndAppend(buf, event);
    tstring sz = buf.str();
    const tchar * s = sz.c_str();

    BOOL bSuccess = ::ReportEvent(hEventLog,
                                  getEventType(event),
                                  getEventCategory(event),
                                  0x1000,
                                  pCurrentUserSID,
                                  1,
                                  0,
                                  &s,
                                  NULL);

    if(!bSuccess) {
        getLogLog().error(LOG4CPLUS_TEXT("Cannot report event in NT EventLog."));
    }
}




WORD 
NTEventLogAppender::getEventType(const InternalLoggingEvent& event)
{
    WORD ret_val;
    
    switch ((int)event.ll)
    {
    case FATAL_LOG_LEVEL:
    case ERROR_LOG_LEVEL:
        ret_val = EVENTLOG_ERROR_TYPE;
        break;
    case WARN_LOG_LEVEL:
        ret_val = EVENTLOG_WARNING_TYPE;
        break;
    case INFO_LOG_LEVEL:
    case DEBUG_LOG_LEVEL:
    default:
        ret_val = EVENTLOG_INFORMATION_TYPE;
        break;
    }

    return ret_val;
}



WORD 
NTEventLogAppender::getEventCategory(const InternalLoggingEvent& event)
{
    WORD ret_val;
    
    switch (event.ll)
    {
    case FATAL_LOG_LEVEL:
        ret_val = 1;
        break;
    case ERROR_LOG_LEVEL:
        ret_val = 2;
        break;
    case WARN_LOG_LEVEL:
        ret_val = 3;
        break;
    case INFO_LOG_LEVEL:
        ret_val = 4;
        break;
    case DEBUG_LOG_LEVEL:
    default:
        ret_val = 5;
        break;
    }

    return ret_val;
}


// Add this source with appropriate configuration keys to the registry.
void 
NTEventLogAppender::addRegistryInfo()
{
    DWORD disposition;
    HKEY hkey = 0;
    tstring subkey =   LOG4CPLUS_TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\")
                     + log 
                     + LOG4CPLUS_TEXT("\\") 
                     + source;
    
    hkey = regGetKey(subkey, &disposition);
    if(disposition == REG_CREATED_NEW_KEY) {
        regSetString(hkey, 
                     LOG4CPLUS_TEXT("EventMessageFile"), 
                     LOG4CPLUS_TEXT("NTEventLogAppender.dll"));
        regSetString(hkey, 
                     LOG4CPLUS_TEXT("CategoryMessageFile"), 
                     LOG4CPLUS_TEXT("NTEventLogAppender.dll"));
        regSetDword(hkey, LOG4CPLUS_TEXT("TypesSupported"), (DWORD)7);
        regSetDword(hkey, LOG4CPLUS_TEXT("CategoryCount"), (DWORD)5);
    }
    
    RegCloseKey(hkey);
    return;
}



