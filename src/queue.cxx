#include <log4cplus/helpers/queue.h>
#include <log4cplus/helpers/loglog.h>


namespace log4cplus { namespace thread {


Queue::Queue (unsigned len)
    : ev_consumer (false)
    , sem (len, len) 
    , max_len (len)
    , flags (DRAIN)
{ }


Queue::~Queue ()
{ }


unsigned
Queue::put_event (spi::InternalLoggingEvent const & ev)
{
    unsigned ret_flags = ERROR_BIT | ERROR_AFTER;
    try
    {
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


unsigned
Queue::signal_exit (bool drain)
{
    unsigned ret_flags = 0;

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


unsigned
Queue::get_event (spi::InternalLoggingEvent & ev)
{
    unsigned ret_flags = 0;

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
