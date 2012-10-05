// Module:  Log4cplus
// File:    msttsappender.cxx
// Created: 5/2012
// Author:  Vaclav Zeman
//
//
//  Copyright (C) 2012, Vaclav Zeman. All rights reserved.
//  
//  Redistribution and use in source and binary forms, with or without modifica-
//  tion, are permitted provided that the following conditions are met:
//  
//  1. Redistributions of  source code must  retain the above copyright  notice,
//     this list of conditions and the following disclaimer.
//  
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  
//  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED WARRANTIES,
//  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS  FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN NO  EVENT SHALL  THE
//  APACHE SOFTWARE  FOUNDATION  OR ITS CONTRIBUTORS  BE LIABLE FOR  ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLU-
//  DING, BUT NOT LIMITED TO, PROCUREMENT  OF SUBSTITUTE GOODS OR SERVICES; LOSS
//  OF USE, DATA, OR  PROFITS; OR BUSINESS  INTERRUPTION)  HOWEVER CAUSED AND ON
//  ANY  THEORY OF LIABILITY,  WHETHER  IN CONTRACT,  STRICT LIABILITY,  OR TORT
//  (INCLUDING  NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT OF THE  USE OF
//  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <log4cplus/config.hxx>
#include <log4cplus/msttsappender.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/spi/factory.h>
#include <log4cplus/thread/threads.h>
#include <log4cplus/thread/syncprims.h>

#include <iomanip>
#include <sstream>

#include <sapi.h>


namespace log4cplus
{


namespace
{

static
void
loglog_win32_error (tchar const * msg, bool dothrow = false)
{
    DWORD err = GetLastError ();
    tostringstream oss;
    oss << LOG4CPLUS_TEXT ("MSTTSAppender: ") << msg << LOG4CPLUS_TEXT(": ")
        << err << LOG4CPLUS_TEXT (" / 0x")
        << std::setw (8) << std::setfill (LOG4CPLUS_TEXT ('0')) << std::hex
        << err;
    helpers::getLogLog ().error (oss.str (), dothrow);
}


static
void
loglog_com_error (tchar const * msg, HRESULT hr)
{
    tostringstream oss;
    oss << LOG4CPLUS_TEXT ("MSTTSAppender: ") << msg << LOG4CPLUS_TEXT(": ")
        << hr << LOG4CPLUS_TEXT (" / 0x")
        << std::setw (8) << std::setfill (LOG4CPLUS_TEXT ('0')) << std::hex
        << hr;
    helpers::getLogLog ().error (oss.str ());
}


struct COMInitializer
{
    explicit COMInitializer (DWORD apartment_model = COINIT_APARTMENTTHREADED,
        HRESULT hr = S_OK)
        : uninit ((hr = CoInitializeEx (NULL, apartment_model)
            , hr == S_OK || hr == S_FALSE))
    { }
    
    ~COMInitializer ()
    {
        if (uninit)
            CoUninitialize ();
    }

    bool const uninit;
};


class SpeachObjectThread
    : public virtual log4cplus::thread::AbstractThread
{
public:
    SpeachObjectThread (ISpVoice * & ispvoice_ref)
        : ispvoice (ispvoice_ref)
    {
        terminate_ev = CreateEvent (0, true, false, 0);
        if (! terminate_ev)
            loglog_win32_error (
                LOG4CPLUS_TEXT ("SpeachObjectThread: CreateEvent failed"),
                true);
    }


    ~SpeachObjectThread ()
    {
        if (! CloseHandle (terminate_ev))
            loglog_win32_error (
                LOG4CPLUS_TEXT ("SpeachObjectThread: CloseHandle failed"));
    }


    virtual
    void
    run ()
    {
        COMInitializer com_init;
        
        HRESULT hr = CoCreateInstance (CLSID_SpVoice, NULL,
            CLSCTX_ALL, IID_ISpVoice, reinterpret_cast<void **>(&ispvoice));
        if (FAILED (hr))
            loglog_com_error (
                LOG4CPLUS_TEXT ("CoCreateInstance(IID_ISpVoice)"),
                hr);

        init_ev.signal ();

        for (;;)
        {
            // wait for the event and for messages
            DWORD dwReturn = 0;
            hr = ::CoWaitForMultipleHandles (0, INFINITE, 1, &terminate_ev,
                &dwReturn);

            // this thread has been reawakened. Determine why
            // and handle appropriately.
            if (hr == S_OK && dwReturn == WAIT_OBJECT_0)
                // our event happened.
                break;
        }
    };

    void
    wait_init_done () const
    {
        init_ev.wait ();
    }

    void
    terminate () const
    {
        if (! SetEvent (terminate_ev))
            loglog_win32_error (LOG4CPLUS_TEXT ("SetEvent failed"), true);
    }

private:
    ISpVoice * & ispvoice;
    thread::ManualResetEvent init_ev;
    HANDLE terminate_ev;
};


} // namespace


struct MSTTSAppender::Data
{
    Data ()
        : ispvoice (0)
    { }

    ~Data ()
    {
        if (ispvoice)
            ispvoice->Release ();
    }

    helpers::SharedObjectPtr<SpeachObjectThread> speach_thread;
    ISpVoice * ispvoice;
};


MSTTSAppender::MSTTSAppender ()
    : Appender ()
    , data (new Data)
{
    init ();
}


MSTTSAppender::MSTTSAppender (helpers::Properties const & props)
    : Appender (props)
    , data (new Data)
{
    //tstring logname = props.getProperty (LOG4CPLUS_TEXT ("LogName"));

    //unsigned long logsize = MSTTS_APPENDER_INITIAL_LOG_SIZE;
    //props.getULong (logsize, LOG4CPLUS_TEXT ("LogSize"));

    //unsigned long buffersize = MSTTS_APPENDER_DEFAULT_BUFFER_SIZE;
    //props.getULong (buffersize, LOG4CPLUS_TEXT ("BufferSize"));

    init ();
}


MSTTSAppender::~MSTTSAppender ()
{
    destructorImpl ();
    delete data;
}


void
MSTTSAppender::init ()
{
    data->speach_thread = 
        helpers::SharedObjectPtr<SpeachObjectThread> (
            new SpeachObjectThread (data->ispvoice));
    data->speach_thread->start ();
    data->speach_thread->wait_init_done ();
}


void
MSTTSAppender::close ()
{
    data->speach_thread->terminate ();
    data->speach_thread->join ();
}


void
MSTTSAppender::append (spi::InternalLoggingEvent const & ev)
{
    if (! data->ispvoice)
        return;

    // TODO: Expose log4cplus' internal TLS to use here.
    tostringstream oss;
    layout->formatAndAppend(oss, ev);

    tstring str;
    oss.str ().swap (str);

    COMInitializer com_init;
    HRESULT hr = data->ispvoice->Speak (helpers::towstring (str).c_str (),
        SPF_IS_NOT_XML /*| SPF_ASYNC | SPF_NLP_SPEAK_PUNC*/, NULL);
    if (FAILED (hr))
        loglog_com_error (LOG4CPLUS_TEXT ("Speak failed"), hr);
}


void
MSTTSAppender::registerAppender ()
{
    log4cplus::spi::AppenderFactoryRegistry & reg
        = log4cplus::spi::getAppenderFactoryRegistry ();
    LOG4CPLUS_REG_APPENDER (reg, MSTTSAppender);
}


} // namespace log4cplus


extern "C"
BOOL WINAPI DllMain(LOG4CPLUS_DLLMAIN_HINSTANCE,  // handle to DLL module
                    DWORD fdwReason,     // reason for calling function
                    LPVOID)  // reserved
{
    // Perform actions based on the reason for calling.
    switch( fdwReason ) 
    { 
    case DLL_PROCESS_ATTACH:
    {
        log4cplus::MSTTSAppender::registerAppender ();
        break;
    }

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
