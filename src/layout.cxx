// Module:  Log4CPLUS
// File:    layout.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright 2001-2017 Tad E. Smith
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

#include <log4cplus/layout.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/helpers/timehelper.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/internal/internal.h>
#include <ostream>
#include <iomanip>

#if defined (LOG4CPLUS_WITH_UNIT_TESTS)
#include <catch_amalgamated.hpp>
#include <sstream>
#endif


namespace log4cplus
{

void
formatRelativeTimestamp (log4cplus::tostream & output,
    log4cplus::spi::InternalLoggingEvent const & event)
{
    auto const duration
        = event.getTimestamp () - getTTCCLayoutTimeBase ();
    output << helpers::chrono::duration_cast<
                  helpers::chrono::duration<long long, std::milli>>(
                      duration).count ();
}

//
//
//


Layout::Layout ()
    : llmCache(getLogLevelManager())
{ }


Layout::Layout (const log4cplus::helpers::Properties& properties)
    : Layout()
{
    if (! properties.exists(LOG4CPLUS_TEXT("EOL")))
        return;

    auto const name = helpers::toUpper(
        properties.getProperty(LOG4CPLUS_TEXT("EOL")));
    static constexpr struct
    {
        tstring_view name;
        EndOfLine value;
    } values[] = {
        {LOG4CPLUS_TEXT("CR"), EndOfLine::CR},
        {LOG4CPLUS_TEXT("LF"), EndOfLine::LF},
        {LOG4CPLUS_TEXT("CRLF"), EndOfLine::CRLF},
        {LOG4CPLUS_TEXT("NEL"), EndOfLine::NEL},
        {LOG4CPLUS_TEXT("LS"), EndOfLine::LS},
        {LOG4CPLUS_TEXT("PS"), EndOfLine::PS},
    };
    for (auto const & value : values)
    {
        if (name == value.name)
        {
            setEOL(value.value);
            return;
        }
    }

    helpers::getLogLog().warn(LOG4CPLUS_TEXT("Invalid EOL property: \"")
        + properties.getProperty(LOG4CPLUS_TEXT("EOL"))
        + LOG4CPLUS_TEXT("\"; using LF"));
}


Layout::~Layout() = default;


void
Layout::setEOL(EndOfLine value)
{
    switch (value)
    {
    case EndOfLine::CR:
        eolString = LOG4CPLUS_TEXT("\r");
        break;
    case EndOfLine::CRLF:
        eolString = LOG4CPLUS_TEXT("\r\n");
        break;
    case EndOfLine::NEL:
#if defined (UNICODE)
        eolString = L"\u0085";
#else
        eolString = "\xC2\x85";
#endif
        break;
    case EndOfLine::LS:
#if defined (UNICODE)
        eolString = L"\u2028";
#else
        eolString = "\xE2\x80\xA8";
#endif
        break;
    case EndOfLine::PS:
#if defined (UNICODE)
        eolString = L"\u2029";
#else
        eolString = "\xE2\x80\xA9";
#endif
        break;
    case EndOfLine::LF:
    default:
        value = EndOfLine::LF;
        eolString = LOG4CPLUS_TEXT("\n");
        break;
    }
    eol = value;
}


EndOfLine
Layout::getEOL() const
{
    return eol;
}


///////////////////////////////////////////////////////////////////////////////
// log4cplus::SimpleLayout public methods
///////////////////////////////////////////////////////////////////////////////

SimpleLayout::SimpleLayout () = default;


SimpleLayout::SimpleLayout (const helpers::Properties& properties)
    : Layout (properties)
{ }


SimpleLayout::~SimpleLayout() = default;


void
SimpleLayout::formatAndAppend(log4cplus::tostream& output,
                              const log4cplus::spi::InternalLoggingEvent& event)
{
    output << llmCache.toString(event.getLogLevel())
           << LOG4CPLUS_TEXT(" - ")
           << event.getMessage()
           << eolString;
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::TTCCLayout ctors and dtor
///////////////////////////////////////////////////////////////////////////////

  TTCCLayout::TTCCLayout(bool use_gmtime_, bool thread_printing_,
      bool category_prefixing_, bool context_printing_)
    : use_gmtime(use_gmtime_)
    , thread_printing (thread_printing_)
    , category_prefixing (category_prefixing_)
    , context_printing (context_printing_)
{
}


TTCCLayout::TTCCLayout(const log4cplus::helpers::Properties& properties)
    : Layout(properties)
    , dateFormat(properties.getProperty (LOG4CPLUS_TEXT("DateFormat"),
            internal::empty_str))
{
    properties.getBool (use_gmtime, LOG4CPLUS_TEXT("Use_gmtime"));
    properties.getBool (thread_printing, LOG4CPLUS_TEXT("ThreadPrinting"));
    properties.getBool (category_prefixing, LOG4CPLUS_TEXT("CategoryPrefixing"));
    properties.getBool (context_printing, LOG4CPLUS_TEXT("ContextPrinting"));
}


TTCCLayout::~TTCCLayout() = default;



///////////////////////////////////////////////////////////////////////////////
// log4cplus::TTCCLayout public methods
///////////////////////////////////////////////////////////////////////////////

void
TTCCLayout::formatAndAppend(log4cplus::tostream& output,
                            const log4cplus::spi::InternalLoggingEvent& event)
{
     if (dateFormat.empty ())
         formatRelativeTimestamp (output, event);
     else
         output << helpers::getFormattedTime(dateFormat, event.getTimestamp(),
             use_gmtime);

     if (getThreadPrinting ())
         output << LOG4CPLUS_TEXT(" [")
                << event.getThread()
                << LOG4CPLUS_TEXT("] ");
     else
         output << LOG4CPLUS_TEXT(' ');

     output << llmCache.toString(event.getLogLevel())
            << LOG4CPLUS_TEXT(' ');

     if (getCategoryPrefixing ())
         output << event.getLoggerName()
                << LOG4CPLUS_TEXT(' ');

     if (getContextPrinting ())
         output << LOG4CPLUS_TEXT("<")
                << event.getNDC()
                << LOG4CPLUS_TEXT("> ");

     output << LOG4CPLUS_TEXT("- ")
            << event.getMessage()
            << eolString;
}


bool
TTCCLayout::getThreadPrinting() const
{
    return thread_printing;
}


void
TTCCLayout::setThreadPrinting(bool thread_printing_)
{
    thread_printing = thread_printing_;
}


bool
TTCCLayout::getCategoryPrefixing() const
{
    return category_prefixing;
}


void
TTCCLayout::setCategoryPrefixing(bool category_prefixing_)
{
    category_prefixing = category_prefixing_;
}


bool
TTCCLayout::getContextPrinting() const
{
    return context_printing;
}


void
TTCCLayout::setContextPrinting(bool context_printing_)
{
    context_printing = context_printing_;
}


#if defined (LOG4CPLUS_WITH_UNIT_TESTS)
namespace
{

std::unique_ptr<Layout>
make_test_layout(int kind, helpers::Properties const * properties = nullptr)
{
    switch (kind)
    {
    case 0:
        if (properties)
            return std::make_unique<SimpleLayout>(*properties);
        return std::make_unique<SimpleLayout>();
    case 1:
        if (properties)
            return std::make_unique<TTCCLayout>(*properties);
        return std::make_unique<TTCCLayout>();
    default:
        if (properties)
            return std::make_unique<PatternLayout>(*properties);
        return std::make_unique<PatternLayout>(LOG4CPLUS_TEXT("%m%n"));
    }
}

} // namespace

CATCH_TEST_CASE("Layouts generate configurable end-of-line separators", "[layout][eol]")
{
    int const kind = GENERATE(0, 1, 2);
    CATCH_CAPTURE(kind);
    spi::InternalLoggingEvent event(LOG4CPLUS_TEXT("logger"), INFO_LOG_LEVEL,
        LOG4CPLUS_TEXT("ctx"), {}, LOG4CPLUS_TEXT("a\nb\r\nc\rd"),
        LOG4CPLUS_TEXT("worker"), LOG4CPLUS_TEXT("worker2"),
        getTTCCLayoutTimeBase() + std::chrono::milliseconds{42},
        LOG4CPLUS_TEXT(""), 0);
    tstring const prefix = kind == 0 ? LOG4CPLUS_TEXT("INFO - ")
        : kind == 1 ? LOG4CPLUS_TEXT("42 [worker] INFO logger <ctx> - ")
        : LOG4CPLUS_TEXT("");
    auto const format = [&] (Layout & layout)
    {
        tostringstream output;
        layout.formatAndAppend(output, event);
        return output.str();
    };

    auto layout = make_test_layout(kind);
    CATCH_CHECK(layout->getEOL() == EndOfLine::LF);
    CATCH_CHECK(format(*layout) == prefix + event.getMessage() + LOG4CPLUS_TEXT("\n"));
    helpers::Properties properties;
    properties.setProperty(LOG4CPLUS_TEXT("ConversionPattern"), LOG4CPLUS_TEXT("%m%n"));
    auto default_layout = make_test_layout(kind, &properties);
    CATCH_CHECK(default_layout->getEOL() == EndOfLine::LF);
    CATCH_CHECK(format(*default_layout) == format(*layout));

    struct
    {
        tchar const * name;
        EndOfLine value;
        tstring_view expected;
    } const cases[] = {
        {LOG4CPLUS_TEXT("cR"), EndOfLine::CR, LOG4CPLUS_TEXT("\r")},
        {LOG4CPLUS_TEXT("lf"), EndOfLine::LF, LOG4CPLUS_TEXT("\n")},
        {LOG4CPLUS_TEXT("CrLf"), EndOfLine::CRLF, LOG4CPLUS_TEXT("\r\n")},
#if defined (UNICODE)
        {L"nel", EndOfLine::NEL, L"\u0085"},
        {L"ls", EndOfLine::LS, L"\u2028"},
        {L"ps", EndOfLine::PS, L"\u2029"},
#else
        {"nel", EndOfLine::NEL, "\xC2\x85"},
        {"ls", EndOfLine::LS, "\xE2\x80\xA8"},
        {"ps", EndOfLine::PS, "\xE2\x80\xA9"},
#endif
    };
    for (auto const & item : cases)
    {
        CATCH_CAPTURE(item.name);
        tstring const expected = prefix + event.getMessage() + tstring(item.expected);
        layout->setEOL(item.value);
        CATCH_CHECK(layout->getEOL() == item.value);
        CATCH_CHECK(format(*layout) == expected);

        properties.setProperty(LOG4CPLUS_TEXT("EOL"), item.name);
        auto configured = make_test_layout(kind, &properties);
        CATCH_CHECK(configured->getEOL() == item.value);
        CATCH_CHECK(format(*configured) == expected);
        CATCH_CHECK(default_layout->getEOL() == EndOfLine::LF);
        CATCH_CHECK(format(*default_layout) == prefix + event.getMessage() + LOG4CPLUS_TEXT("\n"));
    }
}

CATCH_TEST_CASE("Invalid layout EOL properties warn and fall back to LF", "[layout][eol]")
{
    int const kind = GENERATE(0, 1, 2);
    auto const value = GENERATE(LOG4CPLUS_TEXT(""), LOG4CPLUS_TEXT("invalid"));
    CATCH_CAPTURE(kind, value);
    helpers::Properties properties;
    properties.setProperty(LOG4CPLUS_TEXT("ConversionPattern"), LOG4CPLUS_TEXT("%m%n"));
    properties.setProperty(LOG4CPLUS_TEXT("EOL"), value);

    struct CaptureWarnings
    {
        tostringstream output;
        std::basic_streambuf<tchar> * previous = tcerr.rdbuf(output.rdbuf());
        ~CaptureWarnings() { tcerr.rdbuf(previous); }
    } warnings;
    auto layout = make_test_layout(kind, &properties);
    CATCH_CHECK(layout->getEOL() == EndOfLine::LF);
    CATCH_CHECK(warnings.output.str().find(LOG4CPLUS_TEXT("Invalid EOL property:")) != tstring::npos);
    CATCH_CHECK(warnings.output.str().find(LOG4CPLUS_TEXT("using LF")) != tstring::npos);
    tostringstream output;
    layout->formatAndAppend(output, spi::InternalLoggingEvent());
    CATCH_CHECK(output.str().ends_with(LOG4CPLUS_TEXT("\n")));
}

CATCH_TEST_CASE("Custom layouts can use the shared EOL separator", "[layout][eol]")
{
    class CustomLayout : public Layout
    {
    public:
        using Layout::Layout;
        void formatAndAppend(tostream & output,
            spi::InternalLoggingEvent const & event) override
        {
            output << event.getMessage() << eolString;
        }
    };
    helpers::Properties properties;
    properties.setProperty(LOG4CPLUS_TEXT("EOL"), LOG4CPLUS_TEXT("CRLF"));
    CustomLayout layout(properties);
    tostringstream output;
    layout.formatAndAppend(output, spi::InternalLoggingEvent());
    CATCH_CHECK(output.str() == LOG4CPLUS_TEXT("\r\n"));
}
#endif


} // namespace log4cplus
