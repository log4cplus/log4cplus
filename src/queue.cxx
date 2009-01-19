#include <log4cplus/helpers/queue.h>
#include <log4cplus/helpers/loglog.h>


namespace log4cplus { namespace thread {


Queue::Queue (unsigned len)
    : max_len (len)
    , mutex (LOG4CPLUS_MUTEX_CREATE)
    , flags (DRAIN)
{
#if defined (LOG4CPLUS_USE_PTHREADS)
    pthread_cond_init (&cv_consumer, 0);
    pthread_cond_init (&cv_producers, 0);
    pthread_mutex_init (&mtx, 0);

#elif defined (LOG4CPLUS_USE_WIN32_THREADS)
    ev_consumer = CreateEvent (0, true, true, 0);
    sem = CreateSemaphore (0, max_len, max_len, 0);

#else
#  error "This file assumes real threading support."
#endif
}


Queue::~Queue ()
{
    LOG4CPLUS_MUTEX_FREE (mutex);
#if defined (LOG4CPLUS_USE_PTHREADS)
#elif defined (LOG4CPLUS_USE_WIN32_THREADS)
    CloseHandle (ev_consumer);
    CloseHandle (sem);
#endif
}


unsigned
Queue::put_event (spi::InternalLoggingEvent const & ev)
{
    unsigned ret_flags = 0;

    ev.getThread ();
    ev.getNDC ();

#if defined (LOG4CPLUS_USE_PTHREADS)
    // TODO

#elif defined (LOG4CPLUS_USE_WIN32_THREADS)
    DWORD ret = WaitForSingleObject (sem, INFINITE);
    if (ret != WAIT_OBJECT_0)
    {
        getLogLog ().error (
            LOG4CPLUS_TEXT ("Queue::put_event: Error waiting on semaphore."));
        ret_flags |= ERROR_BIT;
        return ret_flags;
    }

    LOG4CPLUS_MUTEX_LOCK (mutex);

    if (flags & EXIT)
    {
        ret = ReleaseSemaphore (sem, 1, 0);
        if (! ret)
        {
            getLogLog ().error (
                LOG4CPLUS_TEXT ("Queue::put_event:")
                LOG4CPLUS_TEXT (" Error releasing semaphore."));
            ret_flags = ERROR_BIT | flags;
            goto leave_cs;
        }
    }
    else
    {
        queue.push_front (ev);
        flags |= QUEUE;
        ret = SetEvent (ev_consumer);
        if (! ret)
        {
            getLogLog ().error (
                LOG4CPLUS_TEXT ("Queue::put_event:")
                LOG4CPLUS_TEXT (" Error signaling queue thread."));
            ret_flags = ERROR_AFTER | flags;
            goto leave_cs;
        }
    }

    ret_flags = flags;
leave_cs:
    LOG4CPLUS_MUTEX_UNLOCK (mutex);
    return ret_flags;

#endif
}


unsigned
Queue::signal_exit (bool drain)
{
    unsigned ret_flags = 0;

#if defined (LOG4CPLUS_USE_PTHREADS)
    // TODO

#elif defined (LOG4CPLUS_USE_WIN32_THREADS)
    LOG4CPLUS_MUTEX_LOCK (mutex);

    if (! (flags & EXIT))
    {
        if (drain)
            flags |= DRAIN;
        else
            flags &= ~DRAIN;
        flags |= EXIT;
        DWORD ret = SetEvent (ev_consumer);
        if (! ret)
        {
            getLogLog ().error (
                LOG4CPLUS_TEXT ("Queue::signal_exit:")
                LOG4CPLUS_TEXT (" Error signaling queue thread."));
            ret_flags |= ERROR_BIT;
        }
    }

    ret_flags |= flags;
    LOG4CPLUS_MUTEX_UNLOCK (mutex);
    return ret_flags;

#endif
}


unsigned
Queue::get_event (spi::InternalLoggingEvent & ev)
{
    unsigned ret_flags = 0;

#if defined (LOG4CPLUS_USE_PTHREADS)
    // TODO
#elif defined (LOG4CPLUS_USE_WIN32_THREADS)
    DWORD ret;

    while (true)
    {
        LOG4CPLUS_MUTEX_LOCK (mutex);

        if (((QUEUE & flags) && ! (EXIT & flags))
            || ((EXIT | DRAIN | QUEUE) & flags) == (EXIT | DRAIN | QUEUE))
        {
            ev.swap (queue.back ());
            queue.pop_back ();
            if (queue.empty ())
                flags &= ~QUEUE;

            ret = ReleaseSemaphore (sem, 1, 0);
            if (! ret)
            {
                getLogLog ().error (
                    LOG4CPLUS_TEXT ("Queue::get_event:")
                    LOG4CPLUS_TEXT (" Error releasing semaphore."));
                ret_flags |= ERROR_AFTER;
            }

            ret_flags |= EVENT;
            goto leave_cs_and_return_flags;
        }
        else if (((EXIT | QUEUE) & flags) == (EXIT | QUEUE))
        {
            size_t const count = queue.size ();
            assert (count != 0);
            queue.clear ();
            flags &= ~QUEUE;

            ret = ResetEvent (ev_consumer);
            if (! ret)
            {
                getLogLog ().error (
                    LOG4CPLUS_TEXT ("Queue::get_event:")
                    LOG4CPLUS_TEXT (" Error signaling queue thread."));
                ret_flags |= ERROR_AFTER;
            }

            ret = ReleaseSemaphore (sem, static_cast<DWORD>(count), 0);
            if (! ret)
            {
                getLogLog ().error (
                    LOG4CPLUS_TEXT ("Queue::get_event:")
                    LOG4CPLUS_TEXT (" Error releasing semaphore."));
                ret_flags |= ERROR_AFTER;
            }

            goto leave_cs_and_return_flags;
        }
        else if (EXIT & flags)
            goto leave_cs_and_return_flags;
        else
        {
            ret = ResetEvent (ev_consumer);
            if (! ret)
            {
                getLogLog ().error (
                    LOG4CPLUS_TEXT ("Queue::get_event:")
                    LOG4CPLUS_TEXT (" Error signaling queue thread."));
                ret_flags |= ERROR_BIT;
                goto leave_cs_and_return_flags;
            }

            LOG4CPLUS_MUTEX_UNLOCK (mutex);
            ret = WaitForSingleObject (ev_consumer, INFINITE);
            if (ret != WAIT_OBJECT_0)
            {
                getLogLog ().error (
                    LOG4CPLUS_TEXT ("Queue::get_event:")
                    LOG4CPLUS_TEXT ("Error waiting on semaphore."));
                ret_flags |= ERROR_BIT;
                return ret_flags;
            }
        }
    }

leave_cs_and_return_flags:
    ret_flags |= flags;
    LOG4CPLUS_MUTEX_UNLOCK (mutex);
    return ret_flags;

#endif
}


} } // namespace log4cplus { namespace thread {
