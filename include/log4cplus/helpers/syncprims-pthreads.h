//   Copyright (C) 2009-2010, Vaclav Haisman. All rights reserved.
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
//! primitives using the POSIX threads. It does not contain any
//! include guards because it is only a fragment to be included by
//! syncprims.h.

#include <limits>
#include <algorithm>


namespace log4cplus { namespace thread {


#define LOG4CPLUS_THROW_RTE(msg) \
    do { detail::syncprims_throw_exception (msg, __FILE__, __LINE__); } while (0)


//
//
//

struct PthreadMutexAttr
{
    PthreadMutexAttr ()
    {
        int ret = pthread_mutexattr_init (&attr);
        if (ret != 0)
            LOG4CPLUS_THROW_RTE ("PthreadMutexAttr::PthreadMutexAttr");
    }


    ~PthreadMutexAttr ()
    try
    {
        int ret = pthread_mutexattr_destroy (&attr);
        if (ret != 0)
            LOG4CPLUS_THROW_RTE ("PthreadMutexAttr::~PthreadMutexAttr");
    }
    catch (...)
    { }


    void
    set_type (Mutex::Type t)
    {
        int mutex_type;
        switch (t)
        {
        case Mutex::RECURSIVE:
            mutex_type = PTHREAD_MUTEX_RECURSIVE;
            break;

        default:
            mutex_type = PTHREAD_MUTEX_DEFAULT;
        }

        int ret = pthread_mutexattr_settype (&attr, mutex_type);
        if (ret != 0)
            LOG4CPLUS_THROW_RTE ("PthreadMutexAttr::set_type");
    }


    pthread_mutexattr_t attr;
};


//
//
//

inline
Mutex::Mutex (Mutex::Type t)
{
    PthreadMutexAttr attr;
    attr.set_type (t);

    int ret = pthread_mutex_init (&mtx, &attr.attr);
    if (ret != 0)
        LOG4CPLUS_THROW_RTE ("Mutex::Mutex");
}


inline
Mutex::~Mutex ()
{
    int ret = pthread_mutex_destroy (&mtx);
    if (ret != 0)
        LOG4CPLUS_THROW_RTE ("Mutex::~Mutex");
}


inline
void
Mutex::lock () const
{
    int ret = pthread_mutex_lock (&mtx);
    if (ret != 0)
        LOG4CPLUS_THROW_RTE ("Mutex::lock");
}


inline
void
Mutex::unlock () const
{
    int ret = pthread_mutex_unlock (&mtx);
    if (ret != 0)
        LOG4CPLUS_THROW_RTE ("Mutex::unlock");
}


//
//
//

inline
Semaphore::Semaphore (unsigned max, unsigned initial)
{
    unsigned const sem_value_max =
#if defined (SEM_VALUE_MAX)
        SEM_VALUE_MAX
#else
        std::numeric_limits<int>::max ()
#endif
        ;

    unsigned const limited_max = (std::min) (max, sem_value_max);
    unsigned const limited_initial = (std::min) (initial, limited_max);
    int ret = sem_init (&sem, 0, limited_initial);
    if (ret != 0)
        LOG4CPLUS_THROW_RTE ("Semaphore::Semaphore");
}


inline
Semaphore::~Semaphore ()
try
{
    int ret = sem_destroy (&sem);
    if (ret != 0)
        LOG4CPLUS_THROW_RTE ("Semaphore::~Semaphore");
}
catch (...)
{ }


inline
void
Semaphore::unlock () const
{
    int ret = sem_post (&sem);
    if (ret != 0)
        LOG4CPLUS_THROW_RTE ("Semaphore::unlock");
}


inline
void
Semaphore::lock () const
{
    int ret = sem_wait (&sem);
    if (ret != 0)
        LOG4CPLUS_THROW_RTE ("Semaphore::lock");
}


//
//
//

inline
ManualResetEvent::ManualResetEvent (bool sig)
    : mtx (Mutex::DEFAULT)
    , sigcount (0)
    , signaled (sig)
{
    int ret = pthread_cond_init (&cv, 0);
    if (ret != 0)
        LOG4CPLUS_THROW_RTE ("ManualResetEvent::ManualResetEvent");
}


inline
ManualResetEvent::~ManualResetEvent ()
try
{
    int ret = pthread_cond_destroy (&cv);
    if (ret != 0)
        LOG4CPLUS_THROW_RTE ("ManualResetEvent::~ManualResetEvent");
}
catch (...)
{ }


inline
void
ManualResetEvent::signal () const
{
    MutexGuard mguard (mtx);

    signaled = true;
    sigcount += 1;
    int ret = pthread_cond_broadcast (&cv);
    if (ret != 0)
        LOG4CPLUS_THROW_RTE ("ManualResetEVent::signal");

}


inline
void
ManualResetEvent::wait () const
{
    MutexGuard mguard (mtx);

    if (! signaled)
    {
        unsigned prev_count = sigcount;
        do
        {
            int ret = pthread_cond_wait (&cv, &mtx.mtx);
            if (ret != 0)
            {
                mguard.unlock ();
                mguard.detach ();
                LOG4CPLUS_THROW_RTE ("ManualResetEvent::wait");
            }
        }
        while (prev_count == sigcount);
    }
}


inline
bool
ManualResetEvent::timed_wait (unsigned long msec) const
{
    MutexGuard mguard (mtx);

    if (! signaled)
    {       
        helpers::Time const wakeup_time (helpers::Time::gettimeofday ()
            + helpers::Time (msec / 1000, (msec % 1000) * 1000));
        struct timespec const ts = {wakeup_time.sec (),
            wakeup_time.usec () * 1000};
        unsigned prev_count = sigcount;
        do
        {
            int ret = pthread_cond_timedwait (&cv, &mtx.mtx, &ts);
            switch (ret)
            {
            case 0:
                break;

            case ETIMEDOUT:
                return false;

            default:
                mguard.unlock ();
                mguard.detach ();
                LOG4CPLUS_THROW_RTE ("ManualResetEvent::timed_wait");
            }
        }
        while (prev_count == sigcount);
    }

    return true;
}


inline
void
ManualResetEvent::reset () const
{
    MutexGuard mguard (mtx);

    signaled = false;
}


#undef LOG4CPLUS_THROW_RTE


} } // namespace log4cplus { namespace thread {
