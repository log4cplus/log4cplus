#ifndef LOG4CPLUS_HELPERS_SOURCE_LOCATION_
#define LOG4CPLUS_HELPERS_SOURCE_LOCATION_

#include <log4cplus/config.hxx>

#if defined (LOG4CPLUS_HAVE_PRAGMA_ONCE)
#pragma once
#endif

#if __has_include (<source_location>)
#  include <source_location>
#endif


namespace log4cplus
{

namespace helpers
{

/**
 * @brief This is a portable equivalent of std::source_location.
 *
 */
class SourceLocation
{
public:
    constexpr SourceLocation (const char* _file = LOG4CPLUS_CALLER_FILE (),
        int _line = LOG4CPLUS_CALLER_LINE (),
        char const * _function = LOG4CPLUS_CALLER_FUNCTION ()) noexcept
        : file_ (_file)
        , function_ (_function)
        , line_ (_line)
    { }

#if defined (__cpp_lib_source_location) && __cpp_lib_source_location >= 201907L
    static consteval SourceLocation current (
        std::source_location _loc = std::source_location::current ()) noexcept;
#else
    static consteval SourceLocation current (
        const char* _file = LOG4CPLUS_CALLER_FILE (),
        int _line = LOG4CPLUS_CALLER_LINE (),
        char const * _function = LOG4CPLUS_CALLER_FUNCTION ()) noexcept;
#endif

    SourceLocation (SourceLocation const &) = default;
    SourceLocation (SourceLocation &&) noexcept = default;

    char const *
    file_name () const noexcept
    {
        return file_;
    }

    char const *
    function_name () const noexcept
    {
        return function_;
    }

    int
    line () const noexcept
    {
        return line_;
    }

private:
    char const * file_;
    char const * function_;
    int line_;
};

#if defined (__cpp_lib_source_location) && __cpp_lib_source_location >= 201907L
consteval SourceLocation
SourceLocation::current (std::source_location loc) noexcept
{
    return SourceLocation {loc.file_name (), loc.line (), loc.function_name ()};
}

#else // #if defined (__cpp_lib_source_location) && __cpp_lib_source_location >= 201907L
consteval SourceLocation
SourceLocation::current (const char* _file, int _line,
    char const * _function) noexcept
{
    return SourceLocation {_file, _line, _function};
}

#endif // #if defined (__cpp_lib_source_location) && __cpp_lib_source_location >= 201907L

} // namespace helpers

} // namespace log4cplus


#endif // LOG4CPLUS_HELPERS_SOURCE_LOCATION_
