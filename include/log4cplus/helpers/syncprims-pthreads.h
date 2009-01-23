//! @file This file contains implementations of synchronization
//! primitives using the POSIX threads. It does not contain any
//! include guards because it is only a fragment to be included by
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
    int ret = pthread_mutex_init (&mtx, 0);
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
    int ret = sem_init (&sem, max, initial);
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
    : sigcount (0)
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
            // TODO
        }
        while (prev_count == sigcount);
    }
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
