//! @file This file contains implementations of synchronization
//! primitives using the Win32 API. It does not contain any include
//! guards because it is only a fragment to be included by
//! syncprims.h.

namespace log4cplus { namespace thread {


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
        throw std::runtime_error ("log4cplus: " __FUNCTION__);
}


inline
void
Semaphore::unlock () const
{
    DWORD ret = ReleaseSemaphore (sem, 1, 0);
    if (! ret)
        throw std::runtime_error ("log4cplus: " __FUNCTION__);
}


inline
void
Semaphore::lock () const
{
    DWORD ret = WaitForSingleObject (sem, INFINITE);
    if (ret != WAIT_OBJECT_0)
        throw std::runtime_error ("log4cplus: " __FUNCTION__);
}


//
//
//

inline
ManualResetEvent::ManualResetEvent (bool sig)
{
    ev = CreateEvent (0, true, sig, 0);
    if (! ev)
        throw std::runtime_error ("log4cplus: " __FUNCTION__);
}


inline
void
ManualResetEvent::signal () const
{
    if (! SetEvent (ev))
        throw std::runtime_error ("log4cplus: " __FUNCTION__);
}


inline
void
ManualResetEvent::wait () const
{
    DWORD ret = WaitForSingleObject (ev, INFINITE);
    if (ret != WAIT_OBJECT_0)
        throw std::runtime_error ("log4cplus: " __FUNCTION__);
}


inline
void
ManualResetEvent::reset () const
{
    if (! ResetEvent (ev))
        throw std::runtime_error ("log4cplus: " __FUNCTION__);
}


} } // namespace log4cplus { namespace thread {
