//   Copyright (C) 2009, Vaclav Haisman. All rights reserved.
//   
//   Redistribution and use in source and binary forms, with or without modifica-
//   tion, are permitted provided that the following conditions are met:
//   
//   1. Redistributions of  source code must  retain the above copyright  notice,
//      this list of conditions and the following disclaimer.
//   
//   2. Redistributions in binary form must reproduce the above copyright notice,
//      this list of conditions and the following disclaimer in the documentation
//      and/or other materials provided with the distribution.
//   
//   THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED WARRANTIES,
//   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//   FITNESS  FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN NO  EVENT SHALL  THE
//   APACHE SOFTWARE  FOUNDATION  OR ITS CONTRIBUTORS  BE LIABLE FOR  ANY DIRECT,
//   INDIRECT, INCIDENTAL, SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLU-
//   DING, BUT NOT LIMITED TO, PROCUREMENT  OF SUBSTITUTE GOODS OR SERVICES; LOSS
//   OF USE, DATA, OR  PROFITS; OR BUSINESS  INTERRUPTION)  HOWEVER CAUSED AND ON
//   ANY  THEORY OF LIABILITY,  WHETHER  IN CONTRACT,  STRICT LIABILITY,  OR TORT
//   (INCLUDING  NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT OF THE  USE OF
//   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//! @file
//! This file contains implementations of synchronization
//! primitives using the Win32 API. It does not contain any include
//! guards because it is only a fragment to be included by
//! syncprims.h.

namespace log4cplus { namespace thread {


#define LOG4CPLUS_THROW_RTE(msg) \
    do { detail::syncprims_throw_exception (msg, __FILE__, __LINE__); } while (0)

//
//
//

inline
Mutex::Mutex (Mutex::Type)
{
    InitializeCriticalSection (&cs);
}


inline
Mutex::~Mutex ()
{
    DeleteCriticalSection (&cs);
}


inline
void
Mutex::lock () const
{
    EnterCriticalSection (&cs);
}


inline
void
Mutex::unlock () const
{
    LeaveCriticalSection (&cs);
}


//
//
//

inline
Semaphore::Semaphore (unsigned max, unsigned initial)
{
    sem = CreateSemaphore (0, initial, max, 0);
    if (! sem)
        LOG4CPLUS_THROW_RTE ("Semaphore::Semaphore");
}


inline
Semaphore::~Semaphore ()
{
    try
    {
        if (! CloseHandle (sem))
            LOG4CPLUS_THROW_RTE ("Semaphore::~Semaphore");
    }
    catch (...)
    { }
}


inline
void
Semaphore::unlock () const
{
    DWORD ret = ReleaseSemaphore (sem, 1, 0);
    if (! ret)
         LOG4CPLUS_THROW_RTE ("Semaphore::unlock");
}


inline
void
Semaphore::lock () const
{
    DWORD ret = WaitForSingleObject (sem, INFINITE);
    if (ret != WAIT_OBJECT_0)
        LOG4CPLUS_THROW_RTE ("Semaphore::lock");
}


//
//
//

inline
ManualResetEvent::ManualResetEvent (bool sig)
{
    ev = CreateEvent (0, true, sig, 0);
    if (! ev)
        LOG4CPLUS_THROW_RTE ("ManualResetEvent::ManualResetEvent");
}


inline
ManualResetEvent::~ManualResetEvent ()
{
    try
    {
        if (! CloseHandle (ev))
            LOG4CPLUS_THROW_RTE ("ManualResetEvent::~ManualResetEvent");
    }
    catch (...)
    { }
}


inline
void
ManualResetEvent::signal () const
{
    if (! SetEvent (ev))
        LOG4CPLUS_THROW_RTE ("ManualResetEVent::signal");
}


inline
void
ManualResetEvent::wait () const
{
    DWORD ret = WaitForSingleObject (ev, INFINITE);
    if (ret != WAIT_OBJECT_0)
        LOG4CPLUS_THROW_RTE ("ManualResetEvent::wait");
}


inline
bool
ManualResetEvent::timed_wait (unsigned long msec) const
{
    DWORD ret = WaitForSingleObject (ev, static_cast<DWORD>(msec));
    switch(ret)
    {
    case WAIT_OBJECT_0:
        return true;

    case WAIT_TIMEOUT:
        return false;

    default:
        LOG4CPLUS_THROW_RTE ("ManualResetEvent::timed_wait");
        // Silence warnings about not returning any value from function
        // returning bool.
        return false;
    }
}


inline
void
ManualResetEvent::reset () const
{
    if (! ResetEvent (ev))
        LOG4CPLUS_THROW_RTE ("ManualResetEvent::reset");
}


#undef LOG4CPLUS_THROW_RTE


} } // namespace log4cplus { namespace thread {
