// Module:  Log4CPLUS
// File:    loggingmacros.cxx
// Created: 4/2010
// Author:  Vaclav Haisman
//
//
// Copyright 2010-2017 Vaclav Haisman
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/** @file
 * This file implements support function for loggingmacros.h file. */

#include <log4cplus/internal/internal.h>
#include <log4cplus/loggingmacros.h>

#if defined (LOG4CPLUS_WITH_UNIT_TESTS)
#include <catch_amalgamated.hpp>
#include <log4cplus/callbackappender.h>
#include <log4cplus/configurator.h>
#include <log4cplus/hierarchy.h>
#include <atomic>
#include <thread>
#endif


namespace log4cplus::detail {


//! Helper stream to get the defaults from.
static tostringstream const macros_oss_defaults;

// Individual defaults.

static std::ios_base::fmtflags const default_flags
    = macros_oss_defaults.flags ();
static log4cplus::tchar const default_fill = macros_oss_defaults.fill ();
static std::streamsize const default_precision
    = macros_oss_defaults.precision ();
static std::streamsize const default_width = macros_oss_defaults.width ();

//! Clears string stream using defaults taken from macros_oss_defaults.
void
clear_tostringstream (tostringstream & os)
{
    os.clear ();
    os.str (internal::empty_str);
    os.setf (default_flags);
    os.fill (default_fill);
    os.precision (default_precision);
    os.width (default_width);
#if defined (LOG4CPLUS_WORKING_LOCALE)
    std::locale glocale = std::locale ();
    if (os.getloc () != glocale)
        os.imbue (glocale);
#endif // defined (LOG4CPLUS_WORKING_LOCALE)
}


log4cplus::tostringstream &
get_macro_body_oss ()
{
    tostringstream & oss = internal::get_ptd ()->macros_oss;
    clear_tostringstream (oss);
    return oss;
}


log4cplus::helpers::snprintf_buf &
get_macro_body_snprintf_buf ()
{
    return internal::get_ptd ()->snprintf_buf;
}


void
macro_forced_log (log4cplus::Logger const & logger,
    log4cplus::LogLevel log_level, log4cplus::tchar const * msg,
    char const * filename, int line, char const * func)
{
    macro_forced_log (logger, log_level,
        internal::get_ptd ()->macros_str = msg, filename, line, func);
}


void
macro_forced_log (log4cplus::Logger const & logger,
    log4cplus::LogLevel log_level, log4cplus::tstring_view const & msg,
    char const * filename, int line, char const * func)
{
    log4cplus::spi::InternalLoggingEvent & ev
        = internal::get_ptd ()->forced_log_ev;
    ev.setLoggingEvent (logger.getName (), log_level, msg, filename, line,
        func);
    logger.forcedLog (ev);
}


#if defined (LOG4CPLUS_WITH_UNIT_TESTS)
CATCH_TEST_CASE ("Macros", "[macros]")
{
    CATCH_SECTION ("LOG4CPLUS_MACRO_LOG_LOCATION")
    {
        char const * file = __FILE__;
        // The following variables all have to be on the same line!
        int const line = __LINE__; LOG4CPLUS_MACRO_LOG_LOCATION (loc);
        CATCH_REQUIRE_THAT (loc.file_name (), Catch::Matchers::Equals (file));
        CATCH_REQUIRE (loc.line () == line);
    }
} // CATCH_TEST_CASE

namespace {

struct TraceEvent
{
    tstring message;
    tstring file;
    tstring function;
    LogLevel level;
    int line;
};

struct TraceFixture
{
    TraceLogger::Prefixes saved = TraceLogger::getDefaultPrefixes();
    Hierarchy hierarchy;
    Logger logger = hierarchy.getInstance(LOG4CPLUS_TEXT("trace-prefix-test"));
    std::vector<TraceEvent> events;

    TraceFixture()
    {
        TraceLogger::setDefaultPrefixes({});
        logger.setLogLevel(TRACE_LOG_LEVEL);
        logger.setAdditivity(false);
        logger.addAppender(SharedAppenderPtr(new CallbackAppender(capture, &events)));
    }

    ~TraceFixture()
    {
        TraceLogger::setDefaultPrefixes(std::move(saved));
    }

    static void capture(void * cookie, tchar const * message, tchar const *,
        log4cplus_loglevel_t level, tchar const *, tchar const *,
        unsigned long long, unsigned long, tchar const * file,
        tchar const * function, int line)
    {
        static_cast<std::vector<TraceEvent> *>(cookie)->push_back(
            {message, file, function, static_cast<LogLevel>(level), line});
    }

    void configure(tstring const & text)
    {
        tistringstream input(text);
        PropertyConfigurator config(input, hierarchy,
            PropertyConfigurator::fShadowEnvironment);
        config.configure();
    }
};

} // namespace

CATCH_TEST_CASE("Trace prefixes", "[macros][trace]")
{
    TraceFixture fixture;
    auto const & defaults = TraceLogger::getDefaultPrefixes();

    CATCH_SECTION("default prefixes and caller location")
    {
        int line;
        {
            line = __LINE__; TraceLogger trace(fixture.logger, LOG4CPLUS_TEXT("method"));
        }
        CATCH_REQUIRE(fixture.events.size() == 2);
        CATCH_CHECK(fixture.events[0].message == LOG4CPLUS_TEXT("ENTER: method"));
        CATCH_CHECK(fixture.events[1].message == LOG4CPLUS_TEXT("EXIT:  method"));
        for (auto const & event : fixture.events)
        {
            CATCH_CHECK(event.file == LOG4CPLUS_C_STR_TO_TSTRING(__FILE__));
            CATCH_CHECK(event.line == line);
            CATCH_CHECK_FALSE(event.function.empty());
            CATCH_CHECK(event.level == TRACE_LOG_LEVEL);
        }
    }

    CATCH_SECTION("legacy constructor with exact custom prefixes")
    {
        for (auto const & prefixes : {
            TraceLogger::Prefixes{LOG4CPLUS_TEXT("==> "), LOG4CPLUS_TEXT("<== ")},
            TraceLogger::Prefixes{LOG4CPLUS_TEXT(""), LOG4CPLUS_TEXT("")},
            TraceLogger::Prefixes{LOG4CPLUS_TEXT("  entry\t "), LOG4CPLUS_TEXT("\texit  ")}})
        {
            TraceLogger::setDefaultPrefixes(prefixes);
            fixture.events.clear();
            {
                TraceLogger trace(fixture.logger, LOG4CPLUS_TEXT("method"),
                    "trace-test.cpp", 42, "testFunction");
            }
            CATCH_REQUIRE(fixture.events.size() == 2);
            CATCH_CHECK(fixture.events[0].message == prefixes.enterPrefix + LOG4CPLUS_TEXT("method"));
            CATCH_CHECK(fixture.events[1].message == prefixes.exitPrefix + LOG4CPLUS_TEXT("method"));
            for (auto const & event : fixture.events)
            {
                CATCH_CHECK(event.file == LOG4CPLUS_TEXT("trace-test.cpp"));
                CATCH_CHECK(event.line == 42);
                CATCH_CHECK(event.function == LOG4CPLUS_TEXT("testFunction"));
            }
        }
    }

    CATCH_SECTION("macro preserves its location and ordinary TRACE text")
    {
        TraceLogger::setDefaultPrefixes({LOG4CPLUS_TEXT("==> "), LOG4CPLUS_TEXT("<== ")});
        int line;
        {
            line = __LINE__; LOG4CPLUS_TRACE_METHOD(fixture.logger, LOG4CPLUS_TEXT("method"));
            LOG4CPLUS_TRACE_STR(fixture.logger, LOG4CPLUS_TEXT("ordinary trace"));
        }
        CATCH_REQUIRE(fixture.events.size() == 3);
        CATCH_CHECK(fixture.events[0].message == LOG4CPLUS_TEXT("==> method"));
        CATCH_CHECK(fixture.events[1].message == LOG4CPLUS_TEXT("ordinary trace"));
        CATCH_CHECK(fixture.events[2].message == LOG4CPLUS_TEXT("<== method"));
        CATCH_CHECK(fixture.events[0].line == line);
        CATCH_CHECK(fixture.events[2].line == line);
        CATCH_CHECK(fixture.events[0].file == LOG4CPLUS_C_STR_TO_TSTRING(__FILE__));
        CATCH_CHECK(fixture.events[2].function == fixture.events[0].function);
    }

    CATCH_SECTION("active and nested scopes retain their pair")
    {
        {
            TraceLogger outer(fixture.logger, LOG4CPLUS_TEXT("outer"));
            TraceLogger::setDefaultPrefixes({LOG4CPLUS_TEXT("==> "), LOG4CPLUS_TEXT("<== ")});
            auto const & arrows = TraceLogger::getDefaultPrefixes();
            {
                TraceLogger inner(fixture.logger, LOG4CPLUS_TEXT("inner"));
                TraceLogger::setDefaultPrefixes({LOG4CPLUS_TEXT("[ "), LOG4CPLUS_TEXT("] ")});
            }
            CATCH_CHECK(arrows.enterPrefix == LOG4CPLUS_TEXT("==> "));
            CATCH_CHECK(arrows.exitPrefix == LOG4CPLUS_TEXT("<== "));
        }
        {
            TraceLogger next(fixture.logger, LOG4CPLUS_TEXT("next"));
        }
        CATCH_REQUIRE(fixture.events.size() == 6);
        CATCH_CHECK(fixture.events[0].message == LOG4CPLUS_TEXT("ENTER: outer"));
        CATCH_CHECK(fixture.events[1].message == LOG4CPLUS_TEXT("==> inner"));
        CATCH_CHECK(fixture.events[2].message == LOG4CPLUS_TEXT("<== inner"));
        CATCH_CHECK(fixture.events[3].message == LOG4CPLUS_TEXT("EXIT:  outer"));
        CATCH_CHECK(fixture.events[4].message == LOG4CPLUS_TEXT("[ next"));
        CATCH_CHECK(fixture.events[5].message == LOG4CPLUS_TEXT("] next"));
        TraceLogger::setDefaultPrefixes({});
        CATCH_CHECK(&TraceLogger::getDefaultPrefixes() == &defaults);
    }

    CATCH_SECTION("unchanged settings reuse their snapshot and long strings remain valid")
    {
        tstring const enter(256, LOG4CPLUS_TEXT('E'));
        tstring const exit(256, LOG4CPLUS_TEXT('X'));
        TraceLogger::setDefaultPrefixes({enter, exit});
        auto const & snapshot = TraceLogger::getDefaultPrefixes();
        auto const * const data = snapshot.enterPrefix.data();
        TraceLogger::setDefaultPrefixes({enter, exit});
        CATCH_CHECK(&TraceLogger::getDefaultPrefixes() == &snapshot);
        TraceLogger::setDefaultPrefixes({});
        CATCH_CHECK(snapshot.enterPrefix.data() == data);
        CATCH_CHECK(snapshot.enterPrefix == enter);
        CATCH_CHECK(snapshot.exitPrefix == exit);
    }

    CATCH_SECTION("TRACE stays disabled")
    {
        fixture.logger.setLogLevel(INFO_LOG_LEVEL);
        { TraceLogger trace(fixture.logger, LOG4CPLUS_TEXT("method")); }
        CATCH_CHECK(fixture.events.empty());
    }

    CATCH_SECTION("TRACE disabled before exit")
    {
        {
            TraceLogger trace(fixture.logger, LOG4CPLUS_TEXT("method"));
            fixture.logger.setLogLevel(INFO_LOG_LEVEL);
        }
        CATCH_REQUIRE(fixture.events.size() == 1);
        CATCH_CHECK(fixture.events[0].message == LOG4CPLUS_TEXT("ENTER: method"));
    }

    CATCH_SECTION("TRACE enabled before exit retains the construction-time pair")
    {
        fixture.logger.setLogLevel(INFO_LOG_LEVEL);
        {
            TraceLogger trace(fixture.logger, LOG4CPLUS_TEXT("method"));
            TraceLogger::setDefaultPrefixes({LOG4CPLUS_TEXT("==> "), LOG4CPLUS_TEXT("<== ")});
            fixture.logger.setLogLevel(TRACE_LOG_LEVEL);
        }
        CATCH_REQUIRE(fixture.events.size() == 1);
        CATCH_CHECK(fixture.events[0].message == LOG4CPLUS_TEXT("EXIT:  method"));
    }
}

CATCH_TEST_CASE("Trace prefix properties", "[macros][trace][properties]")
{
    TraceFixture fixture;
    fixture.configure(LOG4CPLUS_TEXT(
        "marker===>\r\n"
        "log4cplus.traceLogger.enterPrefix=\"${marker} \"\r\n"
        "log4cplus.traceLogger.exitPrefix=\"<== \"\r\n"));
    auto const & arrows = TraceLogger::getDefaultPrefixes();
    CATCH_CHECK(arrows.enterPrefix == LOG4CPLUS_TEXT("==> "));
    CATCH_CHECK(arrows.exitPrefix == LOG4CPLUS_TEXT("<== "));
    {
        TraceLogger trace(fixture.logger, LOG4CPLUS_TEXT("method"));
    }
    CATCH_REQUIRE(fixture.events.size() == 2);
    CATCH_CHECK(fixture.events[0].message == LOG4CPLUS_TEXT("==> method"));
    CATCH_CHECK(fixture.events[1].message == LOG4CPLUS_TEXT("<== method"));

    fixture.configure(LOG4CPLUS_TEXT("log4cplus.traceLogger.enterPrefix=\n"));
    CATCH_CHECK(TraceLogger::getDefaultPrefixes().enterPrefix.empty());
    CATCH_CHECK(TraceLogger::getDefaultPrefixes().exitPrefix == LOG4CPLUS_TEXT("<== "));
    fixture.configure(LOG4CPLUS_TEXT("log4cplus.traceLogger.exitPrefix=\"\"\n"));
    auto const * const empty = &TraceLogger::getDefaultPrefixes();
    CATCH_CHECK(empty->enterPrefix.empty());
    CATCH_CHECK(empty->exitPrefix.empty());
    fixture.configure(LOG4CPLUS_TEXT("log4cplus.configDebug=false\n"));
    CATCH_CHECK(&TraceLogger::getDefaultPrefixes() == empty);

    fixture.configure(LOG4CPLUS_TEXT(
        "log4cplus.traceLogger.enterPrefix=\"  entry\t \"\n"
        "log4cplus.traceLogger.exitPrefix=  bare  \n"));
    CATCH_CHECK(TraceLogger::getDefaultPrefixes().enterPrefix == LOG4CPLUS_TEXT("  entry\t "));
    CATCH_CHECK(TraceLogger::getDefaultPrefixes().exitPrefix == LOG4CPLUS_TEXT("bare"));
    fixture.configure(LOG4CPLUS_TEXT("log4cplus.traceLogger.enterPrefix=\"unmatched\n"));
    CATCH_CHECK(TraceLogger::getDefaultPrefixes().enterPrefix == LOG4CPLUS_TEXT("\"unmatched"));
    fixture.hierarchy.resetConfiguration();
    CATCH_CHECK(TraceLogger::getDefaultPrefixes().exitPrefix == LOG4CPLUS_TEXT("bare"));
    CATCH_CHECK(arrows.enterPrefix == LOG4CPLUS_TEXT("==> "));
}

#if !defined(LOG4CPLUS_SINGLE_THREADED)
CATCH_TEST_CASE("Concurrent trace prefix publication", "[macros][trace][threads]")
{
    TraceFixture fixture;
    fixture.logger.setLogLevel(INFO_LOG_LEVEL);
    TraceLogger::setDefaultPrefixes({LOG4CPLUS_TEXT("initial"), LOG4CPLUS_TEXT("initial/exit")});
    std::atomic<bool> start{false};
    std::atomic<bool> coherent{true};
    std::vector<std::thread> workers;
    for (int w = 0; w != 2; ++w)
        workers.emplace_back([&, w] {
            while (!start.load(std::memory_order_acquire))
                std::this_thread::yield();
            for (int i = 0; i != 300; ++i)
            {
                tstring enter(64, static_cast<tchar>('A' + (i + w) % 26));
                TraceLogger::setDefaultPrefixes({enter, enter + LOG4CPLUS_TEXT("/exit")});
                std::this_thread::yield();
            }
        });
    for (int r = 0; r != 2; ++r)
        workers.emplace_back([&] {
            while (!start.load(std::memory_order_acquire))
                std::this_thread::yield();
            for (int i = 0; i != 6000; ++i)
            {
                auto const & prefixes = TraceLogger::getDefaultPrefixes();
                TraceLogger trace(fixture.logger, tstring{});
                if (prefixes.exitPrefix != prefixes.enterPrefix + LOG4CPLUS_TEXT("/exit"))
                    coherent.store(false, std::memory_order_relaxed);
            }
        });
    start.store(true, std::memory_order_release);
    for (auto & worker : workers)
        worker.join();
    CATCH_CHECK(coherent.load());
    CATCH_CHECK(fixture.events.empty());
}
#endif

#endif // defined (LOG4CPLUS_WITH_UNIT_TESTS)

} // namespace log4cplus::detail
