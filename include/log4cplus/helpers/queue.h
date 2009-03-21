#ifndef LOG4CPLUS_HELPERS_QUEUE_H
#define LOG4CPLUS_HELPERS_QUEUE_H

#include <deque>
#include <log4cplus/helpers/threads.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/helpers/logloguser.h>
#include <log4cplus/helpers/syncprims.h>


namespace log4cplus { namespace thread {


//! Single consumer, multiple producers queue.
class LOG4CPLUS_EXPORT Queue
    : public virtual helpers::SharedObject
    , protected helpers::LogLogUser
{
public:
    //! Type of the state flags field.
    typedef unsigned flags_type;

    Queue (unsigned len = 100);
    virtual ~Queue ();

    // Producers' methods.

    //! Puts event <code>ev</code> into queue, sets QUEUE flag and
    //! sets internal event object into signaled state. If the EXIT
    //! flags is already set upon entering the function, nothing is
    //! inserted into the queue. The function can block on internal
    //! semaphore if the queue has reached maximal allowed
    //! length. Calling thread is unblocked either by consumer thread
    //! removing item from queue or by any other thread calling
    //! signal_exit().
    //!
    //! \param ev spi::InternalLoggingEvent to be put into the queue.
    //! \return Flags.
    flags_type put_event (spi::InternalLoggingEvent const & ev);

    //! Sets EXIT flag and DRAIN flag and sets internal event object
    //! into signaled state.
    //! \param drain If true, DRAIN flag will be set, otherwise unset.
    //! \return Flags, ERROR_BIT can be set upon error.
    flags_type signal_exit (bool drain = true);

    // Consumer's methods.

    //! The get_event() function is used by queue's consumer. It fills
    //! <code>ev</code> argument using the first item in queue and
    //! sets EVENT flag in return value. If EXIT flag is already set
    //! in flags member upon entering the function then depending on
    //! DRAIN flag it either fills ev argument or does not fill the
    //! argument, if the queue is non-empty. The function blocks by
    //! waiting for internal event object to be signaled if the queue
    //! is empty, unless EXIT flag is set. The calling thread is
    //! unblocked by when items are added into the queue or when exit
    //! is signaled using the signal_exit() function.
    //!
    //! Upon error, return value has one of the error flags set.
    //!
    //! \param ev spi::InternalLoggingEvent instance to be filled from queue.
    //! \return Flags.
    flags_type get_event (spi::InternalLoggingEvent & ev);

    //! Possible state flags.
    enum Flags
    {
        //! EVENT flag is set in return value of get_event() call if
        //! the <code>ev</code> argument is filled with event from the queue.
        EVENT       = 0x0001,

        //! QUEUE flag is set by producers when they put item into the
        //! queue.
        QUEUE       = 0x0002,

        //! EXIT flag is set by signal_exit() call, signaling that the
        //! queue worker thread should end itself.
        EXIT        = 0x0004,

        //! When DRAIN flag is set together with EXIT flag, the queue
        //! worker thread will first drain the queue before exiting.
        DRAIN       = 0x0008,

        //! ERROR_BIT signals error.
        ERROR_BIT   = 0x0010,

        //! ERROR_AFTER signals error that has occured after queue has
        //! already been touched.
        ERROR_AFTER = 0x0020
    };

protected:
    //! Queue storage.
    std::deque<spi::InternalLoggingEvent> queue;

    //! Mutex protecting queue and flags.
    Mutex mutex;

    //! Event on which consumer can wait if it finds queue empty.
    ManualResetEvent ev_consumer;

    //! Semaphore that limits the queue length.
    Semaphore sem;

    //! State flags.
    flags_type flags;

private:
    Queue (Queue const &);
    Queue & operator = (Queue const &);
};


typedef helpers::SharedObjectPtr<Queue> QueuePtr;


} } // namespace log4cplus { namespace thread {



#endif // LOG4CPLUS_HELPERS_QUEUE_H
