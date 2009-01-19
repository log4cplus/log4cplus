#ifndef LOG4CPLUS_HELPERS_QUEUE_H
#define LOG4CPLUS_HELPERS_QUEUE_H

#include <log4cplus/helpers/threads.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/helpers/logloguser.h>


namespace log4cplus { namespace thread {


class LOG4CPLUS_EXPORT Queue
    : public virtual helpers::SharedObject
    , protected helpers::LogLogUser
{
public:
    Queue (unsigned len = 100);
    virtual ~Queue ();

    // Producers' methods.

    unsigned put_event (spi::InternalLoggingEvent const &);
    unsigned signal_exit (bool drain = true);

    // Consumer's methods.

    unsigned get_event (spi::InternalLoggingEvent &);

    enum Flags
    {
        EVENT       = 0x0001,
        QUEUE       = 0x0002,
        EXIT        = 0x0004,
        DRAIN       = 0x0008,
        ERROR_BIT   = 0x0010,
        ERROR_AFTER = 0x0020
    };

protected:
    std::deque<spi::InternalLoggingEvent> queue;
    size_t max_len;

    LOG4CPLUS_MUTEX_PTR_DECLARE mutex;

    unsigned flags;

#if defined (LOG4CPLUS_USE_PTHREADS)
    pthread_cond_t cv_consumer;
    pthread_cond_t cv_producers;
    // POSIX sem_t

#elif defined (LOG4CPLUS_USE_WIN32_THREADS)
    HANDLE ev_consumer;
    HANDLE sem;

#else
#  error "This file assumes real threading support."
#endif

private:
    Queue (Queue const &);
    Queue & operator = (Queue const &);
};


typedef helpers::SharedObjectPtr<Queue> QueuePtr;


} } // namespace log4cplus { namespace thread {



#endif // LOG4CPLUS_HELPERS_QUEUE_H
