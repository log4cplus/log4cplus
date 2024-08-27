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


#include <log4cplus/helpers/eventcounter.h>

namespace log4cplus {

namespace helpers {

BaseEventCounter::BaseEventCounter ()
= default;

BaseEventCounter::~BaseEventCounter ()
= default;

std::size_t
BaseEventCounter::record_event ()
{
    return ++event_count;
}

//
//
//

SteadyClockGate::SteadyClockGate (SteadyClockGate::Duration pause_duration_)
    : pause_duration {pause_duration_}
    , timeout_point {SteadyClockGate::Clock::now ()}
    , prev_timeout_point {timeout_point}
{ }


SteadyClockGate::~SteadyClockGate ()
= default;


bool
SteadyClockGate::latch_open (SteadyClockGate::Info & info)
{
    std::size_t count = event_count.load ();
    if (count == 0)
        return false;

    if (! mtx.try_lock ())
        // Someone else has this locked.
        return false;

    // We have the lock. Attach it to the guard.
    log4cplus::thread::SimpleMutexGuard guard;
    guard.attach (mtx);

    auto const now = Clock::now ();
    if (now >= timeout_point
        // Has anything changed since the first check
        // at the start of the function?
        && (count = event_count.load ()) > 0)
    {
        info.count = count;
        info.time_span = now - prev_timeout_point;
        prev_timeout_point = now;
        timeout_point += pause_duration;

        return true;
    }

    return false;
}


SteadyClockGate::Info::~Info ()
= default;

} // namespace helpers

} // namespace log4cplus