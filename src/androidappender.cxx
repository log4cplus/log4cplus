// Module:  Log4CPLUS
// File:    androidappender.cxx
//
// Copyright 2026 Vaclav Haisman
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

#include <log4cplus/config.hxx>

#if defined (__ANDROID__)

#include <log4cplus/androidappender.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/spi/loggingevent.h>

#include <android/log.h>
#include <cerrno>
#include <cstddef>
#include <string>
#include <string_view>

#if defined (LOG4CPLUS_WITH_UNIT_TESTS)
#include <catch_amalgamated.hpp>
#endif


namespace log4cplus
{

namespace
{

constexpr std::size_t maximum_tag_size = 23;
constexpr std::size_t maximum_message_chunk_size = 4000;
constexpr char default_tag[] = "log4cplus";


bool
is_utf8_continuation (char ch) noexcept
{
    auto const value = static_cast<unsigned char> (ch);
    return (value & 0xc0U) == 0x80U;
}


std::size_t
utf8_prefix_size (std::string_view value, std::size_t maximum_size) noexcept
{
    if (value.size () <= maximum_size)
        return value.size ();

    std::size_t boundary = maximum_size;
    while (boundary > 0 && is_utf8_continuation (value[boundary]))
        --boundary;

    // Preserve malformed input instead of dropping a run of continuation
    // bytes that is longer than the requested prefix.
    return boundary != 0 ? boundary : maximum_size;
}


std::string
normalize_tag (tstring const & value)
{
    std::string result = LOG4CPLUS_TSTRING_TO_STRING (value);
    result.resize (utf8_prefix_size (result, maximum_tag_size));
    if (result.empty ())
        result = default_tag;

    return result;
}


std::size_t
message_chunk_size (std::string_view message, std::size_t offset) noexcept
{
    std::string_view const remaining = message.substr (offset);
    return utf8_prefix_size (remaining, maximum_message_chunk_size);
}


int
android_priority (LogLevel level) noexcept
{
    if (level < DEBUG_LOG_LEVEL)
        return ANDROID_LOG_VERBOSE;
    if (level < INFO_LOG_LEVEL)
        return ANDROID_LOG_DEBUG;
    if (level < WARN_LOG_LEVEL)
        return ANDROID_LOG_INFO;
    if (level < ERROR_LOG_LEVEL)
        return ANDROID_LOG_WARN;
    if (level < FATAL_LOG_LEVEL)
        return ANDROID_LOG_ERROR;
    return ANDROID_LOG_FATAL;
}

} // namespace


AndroidAppender::AndroidAppender (tstring const & tag_)
    : tag (normalize_tag (tag_))
{
}


AndroidAppender::AndroidAppender (helpers::Properties const & properties)
    : Appender (properties)
    , tag (normalize_tag (
          properties.getProperty (LOG4CPLUS_TEXT ("Tag"))))
{
}


AndroidAppender::~AndroidAppender ()
{
    destructorImpl ();
}


void
AndroidAppender::close ()
{
    closed = true;
}


void
AndroidAppender::append (spi::InternalLoggingEvent const & event)
{
    std::string const message
        = LOG4CPLUS_TSTRING_TO_STRING (formatEvent (event));
    int const priority = android_priority (event.getLogLevel ());

    std::size_t offset = 0;
    do
    {
        std::size_t const size = message_chunk_size (message, offset);
        std::string const chunk = message.substr (offset, size);
        int const result
            = __android_log_write (priority, tag.c_str (), chunk.c_str ());
        if (result < 0)
        {
            if (result != -EPERM)
            {
                errorHandler->error (
                    LOG4CPLUS_TEXT ("AndroidAppender: liblog write failed: ")
                    + helpers::convertIntegerToString (result));
            }
            break;
        }

        offset += size;
    }
    while (offset < message.size ());
}


#if defined (LOG4CPLUS_WITH_UNIT_TESTS)

CATCH_TEST_CASE ("AndroidAppender priority mapping", "[android][appender]")
{
    CATCH_CHECK (android_priority (TRACE_LOG_LEVEL) == ANDROID_LOG_VERBOSE);
    CATCH_CHECK (android_priority (DEBUG_LOG_LEVEL) == ANDROID_LOG_DEBUG);
    CATCH_CHECK (android_priority (INFO_LOG_LEVEL) == ANDROID_LOG_INFO);
    CATCH_CHECK (android_priority (WARN_LOG_LEVEL) == ANDROID_LOG_WARN);
    CATCH_CHECK (android_priority (ERROR_LOG_LEVEL) == ANDROID_LOG_ERROR);
    CATCH_CHECK (android_priority (FATAL_LOG_LEVEL) == ANDROID_LOG_FATAL);
    CATCH_CHECK (android_priority (OFF_LOG_LEVEL) == ANDROID_LOG_FATAL);
}


CATCH_TEST_CASE ("AndroidAppender tag normalization", "[android][appender]")
{
    CATCH_CHECK (normalize_tag (tstring ()) == default_tag);
    CATCH_CHECK (normalize_tag (LOG4CPLUS_TEXT ("application"))
        == "application");
    CATCH_CHECK (normalize_tag (LOG4CPLUS_TEXT ("12345678901234567890123"))
        == "12345678901234567890123");

    std::string const tag_with_split_character
        = std::string (22, 'a') + "\xe2\x82\xac";
    CATCH_CHECK (normalize_tag (
        LOG4CPLUS_C_STR_TO_TSTRING (tag_with_split_character.c_str ()))
        == std::string (22, 'a'));
}


CATCH_TEST_CASE ("AndroidAppender message chunking", "[android][appender]")
{
    std::string const ascii (maximum_message_chunk_size * 2 + 1, 'a');
    CATCH_CHECK (message_chunk_size (ascii, 0)
        == maximum_message_chunk_size);
    CATCH_CHECK (message_chunk_size (ascii, maximum_message_chunk_size)
        == maximum_message_chunk_size);
    CATCH_CHECK (message_chunk_size (
        ascii, maximum_message_chunk_size * 2) == 1);

    std::string const utf8
        = std::string (maximum_message_chunk_size - 2, 'a')
        + "\xe2\x82\xac";
    CATCH_CHECK (message_chunk_size (utf8, 0)
        == maximum_message_chunk_size - 2);
    CATCH_CHECK (message_chunk_size (
        utf8, maximum_message_chunk_size - 2) == 3);

    std::string const malformed (maximum_message_chunk_size + 1, '\x80');
    CATCH_CHECK (message_chunk_size (malformed, 0)
        == maximum_message_chunk_size);
}

#endif // defined (LOG4CPLUS_WITH_UNIT_TESTS)

} // namespace log4cplus

#endif // defined (__ANDROID__)
