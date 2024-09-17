// -*- C++ -*-
//
//  Copyright (C) 2024, Vaclav Haisman. All rights reserved.
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

#ifndef LOG4CPLUS_HELPERS_EVENTCOUNTER_H
#define LOG4CPLUS_HELPERS_EVENTCOUNTER_H

#include <log4cplus/config.hxx>

#if defined (LOG4CPLUS_HAVE_PRAGMA_ONCE)
#pragma once
#endif

#include <log4cplus/internal/threadsafetyanalysis.h>
#include <log4cplus/thread/syncprims.h>
#include <cstddef>
#include <atomic>
#include <chrono>


namespace log4cplus {

namespace helpers {

class LOG4CPLUS_EXPORT BaseEventCounter
{
public:
    BaseEventCounter ();
    virtual ~BaseEventCounter ();

    virtual std::size_t record_event ();

protected:
    std::atomic<std::size_t> event_count {0};
};


class LOG4CPLUS_EXPORT SteadyClockGate
    : public BaseEventCounter
{
public:
    using Clock = std::chrono::steady_clock;
    using Duration = Clock::duration;
    using TimePoint = std::chrono::time_point<Clock>;

    struct LOG4CPLUS_EXPORT Info
    {
        ~Info ();

        std::size_t count;
        Duration time_span;
    };

    SteadyClockGate (SteadyClockGate::Duration pause_duraiton);
    virtual ~SteadyClockGate ();

    bool latch_open (Info &);

private:
    log4cplus::thread::SimpleMutex mtx;
    Duration const pause_duration LOG4CPLUS_TSA_GUARDED_BY (mtx);
    TimePoint timeout_point LOG4CPLUS_TSA_GUARDED_BY (mtx);
    TimePoint prev_timeout_point LOG4CPLUS_TSA_GUARDED_BY (mtx);
};


} // namespace helpers

} // namespace log4cplus

#endif // LOG4CPLUS_HELPERS_EVENTCOUNTER_H
