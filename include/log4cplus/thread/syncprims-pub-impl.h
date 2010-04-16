//   Copyright (C) 2010, Vaclav Haisman. All rights reserved.
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

#ifndef LOG4CPLUS_THREAD_SYNCPRIMS_PUB_IMPL_H
#define LOG4CPLUS_THREAD_SYNCPRIMS_PUB_IMPL_H

#include <log4cplus/config.hxx>
#include <log4cplus/thread/syncprims.h>

#if defined (LOG4CPLUS_SINGLE_THREADED)
#  define LOG4CPLUS_THREADED(x)
#else
#  include <log4cplus/thread/impl/syncprims-impl.h>
#  define LOG4CPLUS_THREADED(x) (x)
#endif


namespace log4cplus { namespace thread {


//
//
//

inline
MutexImplBase::~MutexImplBase ()
{ }


//
//
//

inline
Mutex::Mutex ()
    : mtx (LOG4CPLUS_THREADED (new impl::Mutex) + 0)
{ }


inline
Mutex::~Mutex ()
{
    LOG4CPLUS_THREADED (delete static_cast<impl::Mutex *>(mtx));
}


inline
void
Mutex::lock () const
{
    LOG4CPLUS_THREADED (static_cast<impl::Mutex *>(mtx)->lock ());
}


inline
void
Mutex::unlock () const
{
    LOG4CPLUS_THREADED (static_cast<impl::Mutex *>(mtx)->unlock ());
}


//
//
//

inline
SemaphoreImplBase::~SemaphoreImplBase ()
{ }


//
//
//

inline
Semaphore::Semaphore (unsigned max, unsigned initial)
    : sem (LOG4CPLUS_THREADED (new impl::Semaphore (max, initial)) + 0)
{ }


inline
Semaphore::~Semaphore ()
{
    LOG4CPLUS_THREADED (delete static_cast<impl::Semaphore *>(sem));
}


inline
void
Semaphore::lock () const
{
    LOG4CPLUS_THREADED (static_cast<impl::Semaphore *>(sem)->lock ());
}


inline
void
Semaphore::unlock () const
{
    LOG4CPLUS_THREADED (static_cast<impl::Semaphore *>(sem)->unlock ());
}


//
//
//

inline
FairMutexImplBase::~FairMutexImplBase ()
{ }


//
//
//

inline
FairMutex::FairMutex ()
    : mtx (LOG4CPLUS_THREADED (new impl::FairMutex) + 0)
{ }


inline
FairMutex::~FairMutex ()
{
    LOG4CPLUS_THREADED (delete static_cast<impl::FairMutex *>(mtx));
}


inline
void
FairMutex::lock () const
{
    LOG4CPLUS_THREADED (static_cast<impl::FairMutex *>(mtx)->lock ());
}


inline
void
FairMutex::unlock () const
{
    LOG4CPLUS_THREADED (static_cast<impl::FairMutex *>(mtx)->unlock ());
}


//
//
//

inline
ManualResetEventImplBase::~ManualResetEventImplBase ()
{ }


//
//
//

inline
ManualResetEvent::ManualResetEvent (bool sig)
    : ev (LOG4CPLUS_THREADED (new impl::ManualResetEvent (sig)) + 0)
{ }


inline
ManualResetEvent::~ManualResetEvent ()
{
    LOG4CPLUS_THREADED (delete static_cast<impl::ManualResetEvent *>(ev));
}


inline
void
ManualResetEvent::signal () const
{
    LOG4CPLUS_THREADED (static_cast<impl::ManualResetEvent *>(ev)->signal ());
}


inline
void
ManualResetEvent::wait () const
{
    LOG4CPLUS_THREADED (static_cast<impl::ManualResetEvent *>(ev)->wait ());
}


inline
bool
ManualResetEvent::timed_wait (unsigned long msec) const
{
#if defined (LOG4CPLUS_SINGLE_THREADED)
    return true;
#else
    return static_cast<impl::ManualResetEvent *>(ev)->timed_wait (msec);
#endif
}


inline
void
ManualResetEvent::reset () const
{
    LOG4CPLUS_THREADED (static_cast<impl::ManualResetEvent *>(ev)->reset ());
}


} } // namespace log4cplus { namespace thread { 



#endif // LOG4CPLUS_THREAD_SYNCPRIMS_PUB_IMPL_H
