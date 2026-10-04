//  Copyright (C) 2009-2017, Vaclav Haisman. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without modifica-
//  tion, are permitted provided that the following conditions are met:
//
//  1. Redistributions of  source code must  retain the above copyright  notice,
//     this list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED WARRANTIES,
//  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS  FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN NO  EVENT SHALL  THE
//  APACHE SOFTWARE  FOUNDATION  OR ITS CONTRIBUTORS  BE LIABLE FOR  ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLU-
//  DING, BUT NOT LIMITED TO, PROCUREMENT  OF SUBSTITUTE GOODS OR SERVICES; LOSS
//  OF USE, DATA, OR  PROFITS; OR BUSINESS  INTERRUPTION)  HOWEVER CAUSED AND ON
//  ANY  THEORY OF LIABILITY,  WHETHER  IN CONTRACT,  STRICT LIABILITY,  OR TORT
//  (INCLUDING  NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT OF THE  USE OF
//  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <log4cplus/config.hxx>
#ifndef LOG4CPLUS_SINGLE_THREADED

#if defined (LOG4CPLUS_WITH_UNIT_TESTS)
// Include full Windows.h early for Catch.
#include <log4cplus/config/windowsh-inc-full.h>
#include <catch.hpp>
#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>
#endif

#include <log4cplus/asyncappender.h>
#include <log4cplus/spi/factory.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/thread/syncprims-pub-impl.h>


namespace log4cplus
{


namespace
{


class QueueThread
    : public thread::AbstractThread
{
public:
    QueueThread (AsyncAppenderPtr, thread::QueuePtr);

    void run() override;

private:
    AsyncAppenderPtr appenders;
    thread::QueuePtr queue;
};


QueueThread::QueueThread (AsyncAppenderPtr aai, thread::QueuePtr q)
    : appenders (std::move (aai))
    , queue (std::move (q))
{ }


void
QueueThread::run()
{
    typedef log4cplus::thread::Queue::queue_storage_type ev_buf_type;
    ev_buf_type ev_buf;

    while (true)
    {
        unsigned qflags = queue->get_events (&ev_buf);
        if (qflags & thread::Queue::EVENT)
        {
            ev_buf_type::const_iterator const ev_buf_end = ev_buf.end ();
            for (ev_buf_type::const_iterator it = ev_buf.begin ();
                it != ev_buf_end; ++it)
                appenders->appendLoopOnAppenders (*it);
        }

        if (((thread::Queue::EXIT | thread::Queue::DRAIN
                | thread::Queue::EVENT) & qflags)
            == (thread::Queue::EXIT | thread::Queue::DRAIN
                | thread::Queue::EVENT))
            continue;
        else if (thread::Queue::EXIT & qflags)
            break;
    }
}


} // namespace


AsyncAppender::AsyncAppender (SharedAppenderPtr const & app,
    unsigned queue_len)
{
    addAppender (app);
    init_queue_thread (queue_len);
}


AsyncAppender::AsyncAppender (helpers::Properties const & props)
    : Appender (props)
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
        helpers::getLogLog ().error (
            LOG4CPLUS_TEXT ("AsyncAppender::AsyncAppender()")
            LOG4CPLUS_TEXT (" - Cannot find AppenderFactory: ")
            + appender_name, true);
    }

    helpers::Properties appender_props = props.getPropertySubset (
        LOG4CPLUS_TEXT ("Appender."));
    addAppender (factory->createObject (appender_props));

    unsigned queue_len = 100;
    props.getUInt (queue_len, LOG4CPLUS_TEXT ("QueueLimit"));

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
    queue_thread = new QueueThread (AsyncAppenderPtr (this), queue);
    queue_thread->start ();
    helpers::getLogLog ().debug (LOG4CPLUS_TEXT("Queue thread started."));
}


void
AsyncAppender::close ()
{
    if (queue)
    {
        unsigned ret = queue->signal_exit ();
        if (ret & (thread::Queue::ERROR_BIT | thread::Queue::ERROR_AFTER))
            getErrorHandler ()->error (
                LOG4CPLUS_TEXT ("Error in AsyncAppender::close"));
    }

    // isRunning() becomes false before the worker's thread-exit cleanup ends.
    if (queue_thread)
        queue_thread->join ();

    removeAllAppenders();

    queue_thread = nullptr;
    queue = nullptr;
}


void
AsyncAppender::append (spi::InternalLoggingEvent const & ev)
{
    if (queue_thread && queue_thread->isRunning ())
    {
        unsigned ret = queue->put_event (ev);
        if (ret & (thread::Queue::ERROR_BIT | thread::Queue::ERROR_AFTER))
        {
            getErrorHandler ()->error (
                LOG4CPLUS_TEXT ("Error in AsyncAppender::append,")
                LOG4CPLUS_TEXT (" event queue has been lost."));
            // Exit the queue consumer thread without draining
            // the events queue.
            queue->signal_exit (false);
            queue_thread->join ();
            queue_thread = nullptr;
            queue = nullptr;
            appendLoopOnAppenders (ev);
        }
    }
    else
    {
        // If the thread has died for any reason, fall back to synchronous
        // operation.
        appendLoopOnAppenders (ev);
    }
}


#if defined (LOG4CPLUS_WITH_UNIT_TESTS)
namespace
{

struct QueueThreadExitState
{
    std::promise<void> entered;
    std::promise<void> release;
    std::promise<void> helper_ready;
    std::promise<void> close_started;
    std::shared_future<void> release_future = release.get_future ().share ();
    std::atomic<bool> finished {false};
    std::atomic<bool> timed_out {false};
    std::atomic<unsigned> events {0};
    std::atomic<unsigned> synchronous_events {0};
    std::atomic<unsigned> destroyed {0};
    std::thread::id const caller = std::this_thread::get_id ();
};


struct QueueThreadExitGate
{
    std::shared_ptr<QueueThreadExitState> state;

    ~QueueThreadExitGate ()
    {
        state->entered.set_value ();
        state->timed_out = state->release_future.wait_for (
            std::chrono::seconds (10)) != std::future_status::ready;
        state->finished = true;
    }
};


class QueueThreadTestAppender : public Appender
{
public:
    QueueThreadTestAppender (std::shared_ptr<QueueThreadExitState> s, bool fail)
        : state (std::move (s)), fail_first (fail)
    { }

    ~QueueThreadTestAppender () override { destructorImpl (); }
    void close () override { closed = true; }

protected:
    void append (spi::InternalLoggingEvent const &) override
    {
        unsigned const previous = state->events.fetch_add (1);
        if (std::this_thread::get_id () == state->caller)
            ++state->synchronous_events;
        if (previous == 0)
        {
            thread_local QueueThreadExitGate gate {state};
            if (fail_first)
                throw std::runtime_error ("Intentional queue worker failure");
        }
    }

private:
    std::shared_ptr<QueueThreadExitState> state;
    bool const fail_first;
};


class TestAsyncAppender : public AsyncAppender
{
public:
    TestAsyncAppender (SharedAppenderPtr const & sink,
        std::shared_ptr<QueueThreadExitState> s)
        : AsyncAppender (sink, 8), state (std::move (s))
    { }

    explicit TestAsyncAppender (helpers::Properties const & props)
        : AsyncAppender (props)
    { }

    ~TestAsyncAppender () override
    {
        if (state)
            ++state->destroyed;
    }

    thread::AbstractThreadPtr worker () const { return queue_thread; }

private:
    std::shared_ptr<QueueThreadExitState> state;
};


struct QueueThreadCloseFixture
{
    std::shared_ptr<QueueThreadExitState> state
        = std::make_shared<QueueThreadExitState> ();
    helpers::SharedObjectPtr<TestAsyncAppender> appender;
    thread::AbstractThreadPtr worker;
    std::future<void> entered = state->entered.get_future ();
    std::future<void> close_started = state->close_started.get_future ();
    std::promise<void> close_requested;
    std::future<void> closing;
    bool requested = false;
    bool cleaned = false;
    bool cleanup_ok = true;

    explicit QueueThreadCloseFixture (bool fail)
        : appender (new TestAsyncAppender (
              SharedAppenderPtr (new QueueThreadTestAppender (state, fail)),
              state))
        , worker (appender->worker ())
    {
        auto app = appender;
        auto s = state;
        auto request = close_requested.get_future ().share ();
        auto ready = state->helper_ready.get_future ();
        // Start this helper before the worker enters TLS destruction: on
        // Windows, the loader lock can prevent a new thread from starting then.
        closing = std::async (std::launch::async, [app, s, request] {
            s->helper_ready.set_value ();
            if (request.wait_for (std::chrono::seconds (10))
                != std::future_status::ready)
                throw std::runtime_error ("Timed out waiting for close request");
            s->close_started.set_value ();
            app->close ();
        });
        cleanup_ok = ready.wait_for (std::chrono::seconds (10))
            == std::future_status::ready;
    }

    ~QueueThreadCloseFixture () { cleanup (); }

    void append ()
    {
        spi::InternalLoggingEvent event (LOG4CPLUS_TEXT ("queue-close-test"),
            INFO_LOG_LEVEL, LOG4CPLUS_TEXT ("event"), __FILE__, __LINE__);
        appender->doAppend (event);
    }

    void start_close ()
    {
        if (! requested)
        {
            close_requested.set_value ();
            requested = true;
        }
    }

    bool wait_for_exit ()
    {
        return entered.wait_for (std::chrono::seconds (10))
            == std::future_status::ready;
    }

    bool close_is_pending ()
    {
        return close_started.wait_for (std::chrono::seconds (10))
                == std::future_status::ready
            && closing.wait_for (std::chrono::milliseconds (100))
                == std::future_status::timeout;
    }

    // Release and reap everything before assertions, even with the old close()
    // that drops its handle without joining an already stopped worker.
    bool cleanup ()
    {
        if (cleaned)
            return cleanup_ok;
        state->release.set_value ();
        start_close ();
        try
        {
            bool const ready = closing.wait_for (std::chrono::seconds (10))
                == std::future_status::ready;
            cleanup_ok = cleanup_ok && ready;
            closing.get ();
        }
        catch (...)
        {
            cleanup_ok = false;
        }
        try
        {
            worker->join ();
        }
        catch (std::logic_error const &)
        {
            // close() has already joined this retained worker handle.
        }
        catch (...)
        {
            cleanup_ok = false;
        }
        worker = nullptr;
        cleaned = true;
        cleanup_ok = cleanup_ok && ! state->timed_out;
        return cleanup_ok;
    }
};

} // namespace


CATCH_TEST_CASE ("AsyncAppender close waits for failed worker cleanup",
    "[asyncappender][shutdown]")
{
    QueueThreadCloseFixture fixture (true);
    fixture.append ();
    bool const entered = fixture.wait_for_exit ();
    bool const running = fixture.worker->isRunning ();
    fixture.start_close ();
    bool const pending = fixture.close_is_pending ();
    bool const finished = fixture.state->finished;
    bool const cleaned = fixture.cleanup ();

    CATCH_CHECK (entered);
    CATCH_CHECK_FALSE (running);
    CATCH_CHECK (pending);
    CATCH_CHECK_FALSE (finished);
    CATCH_CHECK (cleaned);
    CATCH_CHECK (fixture.state->finished);
}


CATCH_TEST_CASE ("AsyncAppender close drains events and waits for cleanup",
    "[asyncappender][shutdown]")
{
    QueueThreadCloseFixture fixture (false);
    for (unsigned i = 0; i != 4; ++i)
        fixture.append ();
    fixture.start_close ();
    bool const entered = fixture.wait_for_exit ();
    bool const pending = fixture.close_is_pending ();
    unsigned const events = fixture.state->events;
    bool const cleaned = fixture.cleanup ();

    CATCH_CHECK (entered);
    CATCH_CHECK (pending);
    CATCH_CHECK (events == 4);
    CATCH_CHECK (fixture.state->synchronous_events == 0);
    CATCH_CHECK (cleaned);
    CATCH_CHECK (fixture.state->finished);
}


CATCH_TEST_CASE ("AsyncAppender falls back after queue worker failure",
    "[asyncappender][shutdown]")
{
    QueueThreadCloseFixture fixture (true);
    fixture.append ();
    bool const entered = fixture.wait_for_exit ();
    // TLS cleanup starts after isRunning() becomes false. The next append
    // must therefore reach the wrapped appender on this calling thread.
    if (entered)
        fixture.append ();
    bool const cleaned = fixture.cleanup ();

    CATCH_CHECK (entered);
    CATCH_CHECK (fixture.state->events == 2);
    CATCH_CHECK (fixture.state->synchronous_events == 1);
    CATCH_CHECK (cleaned);
}


CATCH_TEST_CASE ("AsyncAppender can close repeatedly and then be destroyed",
    "[asyncappender][shutdown]")
{
    QueueThreadCloseFixture fixture (false);
    fixture.append ();
    fixture.start_close ();
    bool const entered = fixture.wait_for_exit ();
    bool const cleaned = fixture.cleanup ();

    CATCH_CHECK (entered);
    CATCH_CHECK (cleaned);
    CATCH_CHECK_FALSE (fixture.appender->worker ());
    CATCH_CHECK_NOTHROW (fixture.appender->close ());
    fixture.appender = nullptr;
    CATCH_CHECK (fixture.state->destroyed == 1);
}


CATCH_TEST_CASE ("AsyncAppender can close without a queue worker",
    "[asyncappender][shutdown]")
{
    helpers::Properties props;
    helpers::SharedObjectPtr<TestAsyncAppender> appender (
        new TestAsyncAppender (props));
    CATCH_CHECK_FALSE (appender->worker ());
    CATCH_CHECK_NOTHROW (appender->close ());
    CATCH_CHECK_NOTHROW (appender->close ());
    appender = nullptr;
}
#endif // LOG4CPLUS_WITH_UNIT_TESTS


} // namespace log4cplus


#endif // #ifndef LOG4CPLUS_SINGLE_THREADED
