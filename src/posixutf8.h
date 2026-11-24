// SPDX-License-Identifier: Apache-2.0
#ifndef LOG4CPLUS_SRC_POSIX_UTF8_HEADER_
#define LOG4CPLUS_SRC_POSIX_UTF8_HEADER_

#include <log4cplus/internal/utf8.h>
#include <log4cplus/tstring.h>
#include <cerrno>
#include <cwchar>
#include <locale.h>
#ifdef __APPLE__
#include <xlocale.h>
#endif
#include <system_error>

namespace log4cplus::internal
{

LOG4CPLUS_PRIVATE inline void
appendUtf8 (std::string & out, std::uint32_t cp)
{
    if (!helpers::detail::is_scalar (cp))
        cp = helpers::detail::unicode_replacement_character;

    char bytes[helpers::detail::maximum_utf8_sequence_length];
    auto const count = helpers::detail::encode_utf8 (bytes, cp);
    out.append (bytes, count);
}

#ifndef UNICODE
// Only the conversion runs under the saved locale. Layouts, level callbacks
// and error handlers continue to use their caller's locale.
struct LOG4CPLUS_PRIVATE ScopedLocale
{
    explicit ScopedLocale (locale_t loc) : previous (uselocale (loc))
    {
        if (!previous)
            throw std::system_error (errno, std::generic_category (), "uselocale");
    }
    ~ScopedLocale () { uselocale (previous); }
    locale_t previous;
};
#endif

class LOG4CPLUS_PRIVATE PosixUtf8Encoder
{
public:
#ifndef UNICODE
    PosixUtf8Encoder () : locale (duplocale (uselocale (nullptr)))
    {
        if (!locale)
            throw std::system_error (errno, std::generic_category (), "duplocale");
    }
    ~PosixUtf8Encoder () { freelocale (locale); }
    PosixUtf8Encoder (PosixUtf8Encoder const &) = delete;
    PosixUtf8Encoder & operator= (PosixUtf8Encoder const &) = delete;
#endif

    std::string utf8 (tstring_view value) const
    {
        std::string out;
        out.reserve (value.size ());
#ifdef UNICODE
        for (wchar_t cp : value)
            appendUtf8 (out, cp);
#else
        ScopedLocale guard (locale);
        std::mbstate_t state {};
        while (!value.empty ())
        {
            wchar_t cp {};
            auto initial = state;
            auto count = std::mbrtowc (&cp, value.data (), value.size (), &state);
            if (count == static_cast<std::size_t> (-2))
            {
                appendUtf8 (out, helpers::detail::unicode_replacement_character);
                break;
            }
            if (count == static_cast<std::size_t> (-1))
            {
                cp = helpers::detail::unicode_replacement_character;
                // Replace the incomplete prefix as a unit and retry the byte
                // that broke it. A bad initial byte consumes just itself.
                count = 0;
                while (count < value.size ()
                    && std::mbrtowc (nullptr, value.data () + count, 1, &initial)
                        == static_cast<std::size_t> (-2))
                    ++count;
                state = {};
            }
            appendUtf8 (out, cp);
            value.remove_prefix (count ? count : 1);
        }
#endif
        return out;
    }

#ifndef UNICODE
private:
    locale_t locale;
#endif
};

} // namespace log4cplus::internal
#endif
