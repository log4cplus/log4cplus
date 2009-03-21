#include <log4cplus/helpers/queue.h>
#include <log4cplus/helpers/loglog.h>


namespace log4cplus { namespace thread {


Queue::Queue (unsigned len)
    : ev_consumer (false)
    , sem (len, len) 
    , flags (DRAIN)
{ }


Queue::~Queue ()
{ }


Queue::flags_type
Queue::put_event (spi::InternalLoggingEvent const & ev)
{
    flags_type ret_flags = ERROR_BIT;
    try
    {
        // Make sure that the event will have NDC and thread ID of the
        // calling thread and not the queue worker thread.
        ev.getThread ();
        ev.getNDC ();
        
        SemaphoreGuard semguard (sem);
        MutexGuard mguard (mutex);

        ret_flags |= flags;
        
        if (flags & EXIT)
        {
            ret_flags &= ~(ERROR_BIT | ERROR_AFTER);
            return ret_flags;
        }
        else
        {
            queue.push_front (ev);
            ret_flags |= ERROR_AFTER;
            semguard.detach ();
            flags |= QUEUE;
            ret_flags |= flags;
            ev_consumer.signal ();
        }
    }
    catch (std::runtime_error const & e)
    {
        (void)e;
        return ret_flags;
    }

    ret_flags &= ~(ERROR_BIT | ERROR_AFTER);
    return ret_flags;
}


Queue::flags_type
Queue::signal_exit (bool drain)
{
    flags_type ret_flags = 0;

    try
    {
        MutexGuard mguard (mutex);

        ret_flags |= flags;

        if (! (flags & EXIT))
        {
            if (drain)
                flags |= DRAIN;
            else
                flags &= ~DRAIN;
            flags |= EXIT;
            ret_flags = flags;
            ev_consumer.signal ();
        }
    }
    catch (std::runtime_error const & e)
    {
        (void)e;
        ret_flags |= ERROR_BIT;
        return ret_flags;
    }

    return ret_flags;
}


Queue::flags_type
Queue::get_event (spi::InternalLoggingEvent & ev)
{
    flags_type ret_flags = 0;

    try
    {
        while (true)
        {
            MutexGuard mguard (mutex);

            ret_flags = flags;

            if (((QUEUE & flags) && ! (EXIT & flags))
                || ((EXIT | DRAIN | QUEUE) & flags) == (EXIT | DRAIN | QUEUE))
            {
                assert (! queue.empty ());
                ev.swap (queue.back ());
                queue.pop_back ();
                if (queue.empty ())
                    flags &= ~QUEUE;
                sem.unlock ();
                ret_flags = flags | EVENT;
                break;
            }
            else if (((EXIT | QUEUE) & flags) == (EXIT | QUEUE))
            {
                assert (! queue.empty ());
                queue.clear ();
                flags &= ~QUEUE;
                ev_consumer.reset ();
                sem.unlock ();
                ret_flags = flags;
                break;
            }
            else if (EXIT & flags)
                break;
            else
            {
                ev_consumer.reset ();
                mguard.unlock ();
                mguard.detach ();
                ev_consumer.wait ();
            }
        }
    }
    catch (std::runtime_error const & e)
    {
        (void)e;
        ret_flags |= ERROR_BIT;
    }

    return ret_flags;
}


} } // namespace log4cplus { namespace thread {
