//! @file This file contains implementations of synchronization
//! primitives using the Win32 API. It does not contain any include
//! guards because it is only a fragment to be included by
//! syncprims.h.

namespace log4cplus { namespace thread {


#define LOG4CPLUS_THROW_RTE(msg) \
    detail::syncprims_throw_exception (msg, __FILE__, __LINE__);

//
//
//

inline
Mutex::Mutex ()
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
try
{
    if (! CloseHandle (sem))
        LOG4CPLUS_THROW_RTE ("Semaphore::~Semaphore");
}
catch (...)
{ }


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
try
{
    if (! CloseHandle (ev))
        LOG4CPLUS_THROW_RTE ("ManualResetEvent::~ManualResetEvent");
}
catch (...)
{ }


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
void
ManualResetEvent::reset () const
{
    if (! ResetEvent (ev))
        LOG4CPLUS_THROW_RTE ("ManualResetEvent::reset");
}


#undef LOG4CPLUS_THROW_RTE


} } // namespace log4cplus { namespace thread {
