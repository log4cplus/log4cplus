#include <log4cplus/asyncappender.h>
#include <log4cplus/spi/factory.h>
#include <log4cplus/helpers/loglog.h>


namespace log4cplus
{


namespace
{


class QueueThread
    : public thread::AbstractThread
{
public:
    QueueThread (SharedAppenderPtr const &, thread::QueuePtr const &);

    virtual void run();

private:
    SharedAppenderPtr appender;
    thread::QueuePtr queue;
};


QueueThread::QueueThread (SharedAppenderPtr const & app,
    thread::QueuePtr const & q)
    : appender (app)
    , queue (q)
{ }


void
QueueThread::run()
{
    spi::InternalLoggingEvent ev;

    while (true)
    {
        unsigned flags = queue->get_event (ev);

        if (flags & thread::Queue::EVENT)
            appender->doAppend (ev);

        if (((thread::Queue::EXIT | thread::Queue::DRAIN
                | thread::Queue::EVENT) & flags)
            == (thread::Queue::EXIT | thread::Queue::DRAIN
                | thread::Queue::EVENT))
            continue;
        else if (thread::Queue::EXIT & flags)
            break;
    }
}


} // namespace


AsyncAppender::AsyncAppender (SharedAppenderPtr const & app,
    unsigned queue_len)
    : appender (app)
{
    init_queue_thread (queue_len);
}


AsyncAppender::AsyncAppender (helpers::Properties const & props)
{
    tstring const & appender_name (
        props.getProperty (LOG4CPLUS_TEXT ("Appender")));
    if (appender_name.empty ())
    {
        getErrorHandler ()->error (
            LOG4CPLUS_TEXT ("Unspecified appender for AsyncAppender."));
        return;
    }

    spi::AppenderFactoryRegistry & appender_registry
        = spi::getAppenderFactoryRegistry ();
    spi::AppenderFactory * factory = appender_registry.get (appender_name);
    if (! factory)
    {
        tstring const err (LOG4CPLUS_TEXT ("AsyncAppender::AsyncAppender()")
            LOG4CPLUS_TEXT (" - Cannot find AppenderFactory: "));
        getLogLog ().error (err + appender_name);
        return;
    }

    helpers::Properties appender_props = props.getPropertySubset (
        LOG4CPLUS_TEXT ("Appender."));
    appender = factory->createObject (appender_props);

    tstring str (props.getProperty (LOG4CPLUS_TEXT ("QueueLimit"),
        LOG4CPLUS_TEXT ("100")));
    unsigned queue_len = atoi (LOG4CPLUS_TSTRING_TO_STRING (str).c_str ());

    init_queue_thread (queue_len);
}


AsyncAppender::~AsyncAppender ()
{
    destructorImpl ();
}


void
AsyncAppender::init_queue_thread (unsigned queue_len)
{
    queue = new thread::Queue (queue_len);
    queue_thread = new QueueThread (appender, queue);
    queue_thread->start ();
    getLogLog ().debug (LOG4CPLUS_TEXT("Queue thread started."));
}


void
AsyncAppender::doAppend (spi::InternalLoggingEvent const & ev)
{
    append (ev);
}


void
AsyncAppender::close ()
{
    unsigned ret = queue->signal_exit ();
    if (ret & (thread::Queue::ERROR_BIT | thread::Queue::ERROR_AFTER))
        getErrorHandler ()->error (
            LOG4CPLUS_TEXT ("Error in AsyncAppender::close"));
    queue_thread->join ();
}


void
AsyncAppender::append (spi::InternalLoggingEvent const & ev)
{
    unsigned ret = queue->put_event (ev);
    if (ret & (thread::Queue::ERROR_BIT | thread::Queue::ERROR_AFTER))
        getErrorHandler ()->error (
            LOG4CPLUS_TEXT ("Error in AsyncAppender::append"));
}


} // namespace log4cplus
