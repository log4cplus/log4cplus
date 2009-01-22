//! @file This file contains implementations of synchronization
//! primitives using the POSIX threads. It does not contain any
//! include guards because it is only a fragment to be included by
//! syncprims.h.

namespace log4cplus { namespace thread {




#define LOG4CPLUS_THROW_RTE(x) \
    throw std::runtime_error (x)

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
    : signaled (sig)
{
    int ret = pthread_mutex_init (&mtx, 0);
    if (ret != 0)
        LOG4CPLUS_THROW_RTE ("ManualResetEvent::ManualResetEvent");
    ret = pthread_cond_init (&cv, 0);
    if (ret != 0)
        LOG4CPLUS_THROW_RTE ("ManualResetEvent::ManualResetEvent");
}


inline
void
ManualResetEvent::signal () const
{
    int ret = pthread_mutex_lock (&mtx);
    if (ret != 0)
        LOG4CPLUS_THROW_RTE ("ManualResetEVent::signal");
    
    signaled = true;
    sigcount += 1;

    ret = pthread_cond_broadcast (&cv);
    if (ret != 0)
        LOG4CPLUS_THROW_RTE ("ManualResetEVent::signal");
    
    ret = pthread_mutex_unlock (&mtx);
    if (ret != 0)
        LOG4CPLUS_THROW_RTE ("ManualResetEVent::signal");
}


inline
void
ManualResetEvent::wait () const
{
    int ret = pthread_mutex_lock (&mtx);
    if (ret != 0)
        LOG4CPLUS_THROW_RTE ("ManualResetEvent::wait");

    if (! signaled)
    {
        unsigned prev_count = sigcount;
        do
        {
            ret = pthread_cond_wait (&cv, &mtx);
            if (ret != 0)
            {
                ret = pthread_mutex_unlock (&mtx);
                if (ret != 0)
                    LOG4CPLUS_THROW_RTE ("ManualResetEvent::wait");
                LOG4CPLUS_THROW_RTE ("ManualResetEvent::wait");
            }
            // TODO
        }
        while (prev_count == sigcount);
    }

    ret = pthread_mutex_unlock (&mtx);
    if (ret != 0)
        LOG4CPLUS_THROW_RTE ("ManualResetEvent::wait");
}


inline
void
ManualResetEvent::reset () const
{
    int ret = pthread_mutex_lock (&mtx);
    if (ret != 0)
        LOG4CPLUS_THROW_RTE ("ManualResetEvent::reset");

    signaled = false;

    ret = pthread_mutex_unlock (&mtx);
    if (ret != 0)
        LOG4CPLUS_THROW_RTE ("ManualResetEvent::reset");
}


#undef LOG4CPLUS_THROW_RTE


} } // namespace log4cplus { namespace thread {
