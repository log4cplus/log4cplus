#ifndef LOG4CPLUS_THREAD_SYNCPRIMS_H
#define LOG4CPLUS_THREAD_SYNCPRIMS_H

#include <stdexcept>
#include <log4cplus/config.hxx>
#if defined (LOG4CPLUS_USE_PTHREADS)
#  include <pthread.h>
#  include <semaphore.h>

#elif defined (LOG4CPLUS_USE_WIN32_THREADS)
#  undef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>

#endif


namespace log4cplus { namespace thread {


namespace detail
{

LOG4CPLUS_EXPORT void syncprims_throw_exception (char const * const msg,
    char const * const file, int line);

} // namespace detail


template <typename SP>
class SyncGuard
{
public:
    SyncGuard (SP const &);
    ~SyncGuard ();

    void lock ();
    void unlock ();
    void attach (SP const &);
    void detach ();

private:
    SP const * sp;

    SyncGuard (SyncGuard const &);
    SyncGuard & operator = (SyncGuard const &);
};


class ManualResetEvent;


class Mutex
{
public:
    Mutex ();
    ~Mutex ();

    void lock () const;
    void unlock () const;

private:
#if defined (LOG4CPLUS_USE_PTHREADS)
    mutable pthread_mutex_t mtx;
    friend class ManualResetEvent;
#elif defined (LOG4CPLUS_USE_WIN32_THREADS)
    mutable CRITICAL_SECTION cs;
#endif
    
    Mutex (Mutex const &);
    Mutex & operator = (Mutex &);
};


typedef SyncGuard<Mutex> MutexGuard;


class Semaphore
{
public:
    Semaphore (unsigned max, unsigned initial);
    ~Semaphore ();

    void lock () const;
    void unlock () const;

private:
#if defined (LOG4CPLUS_USE_PTHREADS)
    mutable sem_t sem;
#elif defined (LOG4CPLUS_USE_WIN32_THREADS)
    HANDLE sem;
#endif

    Semaphore (Semaphore const &);
    Semaphore & operator = (Semaphore const &);
};


typedef SyncGuard<Semaphore> SemaphoreGuard;


class ManualResetEvent
{
public:
    ManualResetEvent (bool = false);
    ~ManualResetEvent ();

    void signal () const;
    void wait () const;
    void reset () const;

private:
#if defined (LOG4CPLUS_USE_PTHREADS)
    mutable pthread_cond_t cv;
    mutable Mutex mtx;
    mutable volatile unsigned sigcount;
    mutable volatile bool signaled;
#elif defined (LOG4CPLUS_USE_WIN32_THREADS)
    HANDLE ev;
#endif

    ManualResetEvent (ManualResetEvent const &);
    ManualResetEvent & operator = (ManualResetEvent const &);
};


} } // namespace log4cplus { namespace thread {


// Include the appropriate implementations of the classes declared
// above.

#if defined (LOG4CPLUS_USE_PTHREADS)
#  include <log4cplus/helpers/syncprims-pthreads.h>
#elif defined (LOG4CPLUS_USE_WIN32_THREADS)
#  include <log4cplus/helpers/syncprims-win32.h>
#endif


namespace log4cplus { namespace thread {


//
//
//

template <typename SP>
inline
SyncGuard<SP>::SyncGuard (SP const & m)
    : sp (&m)
{
    sp->lock ();
}


template <typename SP>
inline
SyncGuard<SP>::~SyncGuard ()
{
    if (sp)
        sp->unlock ();
}


template <typename SP>
inline
void
SyncGuard<SP>::lock ()
{
    sp->lock ();
}


template <typename SP>
inline
void
SyncGuard<SP>::unlock ()
{
    sp->unlock ();
}


template <typename SP>
inline
void
SyncGuard<SP>::attach (SP const & m)
{
    sp = &m;
}


template <typename SP>
inline
void
SyncGuard<SP>::detach ()
{
    sp = 0;
}


} } // namespace log4cplus { namespace thread {



#endif // LOG4CPLUS_THREAD_SYNCPRIMS_H
