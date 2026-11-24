// SPDX-License-Identifier: Apache-2.0
#include <log4cplus/config.hxx>

#if defined (LOG4CPLUS_WITH_SYSTEMD)

#include <log4cplus/journalappender.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/internal/internal.h>
#include <log4cplus/internal/utf8.h>

#define SD_JOURNAL_SUPPRESS_LOCATION
#include <systemd/sd-journal.h>

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cwchar>
#include <locale.h>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <system_error>
#include <sys/uio.h>

namespace log4cplus
{
namespace
{

void
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
struct ScopedLocale
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

class Encoder
{
public:
#ifndef UNICODE
    Encoder () : locale (duplocale (uselocale (nullptr)))
    {
        if (!locale)
            throw std::system_error (errno, std::generic_category (), "duplocale");
    }
    ~Encoder () { freelocale (locale); }
    Encoder (Encoder const &) = delete;
    Encoder & operator= (Encoder const &) = delete;
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

bool
validFieldName (tstring_view name)
{
    auto upper = [] (tchar c) { return c >= 'A' && c <= 'Z'; };
    return !name.empty () && name.size () <= 64 && upper (name.front ())
        && std::all_of (name.begin (), name.end (), [upper] (tchar c) {
            return upper (c) || (c >= '0' && c <= '9') || c == '_';
        });
}

} // namespace

struct JournalAppender::Options
{
    Encoder encoder;
    std::string identifier;
    std::vector<std::pair<tstring, std::string>> mappings;

    explicit Options (helpers::Properties const & props)
        : identifier (encoder.utf8 (props.getProperty (LOG4CPLUS_TEXT ("Identifier"))))
    {
        std::set<std::string> used {
            "MESSAGE", "PRIORITY", "SYSLOG_IDENTIFIER", "CODE_FILE",
            "CODE_LINE", "CODE_FUNC", "LOG4CPLUS_MESSAGE", "LOG4CPLUS_LEVEL",
            "LOG4CPLUS_LEVEL_VALUE", "LOG4CPLUS_TIMESTAMP_USEC", "LOG4CPLUS_LOGGER",
            "LOG4CPLUS_THREAD", "LOG4CPLUS_THREAD2", "LOG4CPLUS_NDC"
        };
        auto const subset = props.getPropertySubset (LOG4CPLUS_TEXT ("MDCField."));
        for (auto const & key : subset.propertyNames ())
        {
            auto const & field = subset.getProperty (key);
            if (key.empty () || !validFieldName (field))
                throw std::invalid_argument ("Invalid journal mapping MDCField."
                    + encoder.utf8 (key) + "=" + encoder.utf8 (field));
            std::string name (field.begin (), field.end ());
            if (!used.insert (name).second)
                throw std::invalid_argument ("Journal field already assigned: MDCField."
                    + encoder.utf8 (key) + "=" + name);
            mappings.emplace_back (key, std::move (name));
        }
    }
};

JournalAppender::JournalAppender ()
    : JournalAppender (helpers::Properties ())
{ }

JournalAppender::JournalAppender (helpers::Properties const & props)
    : JournalAppender (props, std::make_unique<Options> (props))
{ }

JournalAppender::JournalAppender (helpers::Properties const & props,
    std::unique_ptr<Options const> opts)
    : Appender (props)
    , options (std::move (opts))
{ }

JournalAppender::~JournalAppender ()
{
    destructorImpl ();
}

void
JournalAppender::close ()
{
    thread::MutexGuard guard (access_mutex);
    closed = true;
}

void
JournalAppender::append (spi::InternalLoggingEvent const & event)
{
    std::vector<std::string> fields;
    fields.reserve (14 + options->mappings.size ());
    auto addText = [&] (std::string fieldName, tstring const & value) {
        fieldName += '=';
        fieldName += options->encoder.utf8 (value);
        fields.push_back (std::move (fieldName));
    };
    auto addOptional = [&] (char const * fieldName, tstring const & value) {
        if (!value.empty ())
            addText (fieldName, value);
    };

    addText ("MESSAGE", formatEvent (event));
    addText ("LOG4CPLUS_MESSAGE", event.getMessage ());
    addText ("LOG4CPLUS_LOGGER", event.getLoggerName ());
    auto level = event.getLogLevel ();
    addText ("LOG4CPLUS_LEVEL", getLogLevelManager ().toString (level));
    fields.push_back ("PRIORITY=" + std::to_string (internal::getSyslogPriority (level)));
    fields.push_back ("LOG4CPLUS_LEVEL_VALUE=" + std::to_string (level));
    fields.push_back ("LOG4CPLUS_TIMESTAMP_USEC="
        + std::to_string (event.getTimestamp ().time_since_epoch ().count ()));
    if (!options->identifier.empty ())
        fields.push_back ("SYSLOG_IDENTIFIER=" + options->identifier);
    addOptional ("CODE_FILE", event.getFile ());
    if (event.getLine () > 0)
        fields.push_back ("CODE_LINE=" + std::to_string (event.getLine ()));
    addOptional ("CODE_FUNC", event.getFunction ());
    addOptional ("LOG4CPLUS_THREAD", event.getThread ());
    addOptional ("LOG4CPLUS_THREAD2", event.getThread2 ());
    addOptional ("LOG4CPLUS_NDC", event.getNDC ());
    for (auto const & [key, fieldName] : options->mappings)
    {
        auto const & mdc = event.getMDCCopy ();
        if (auto it = mdc.find (key); it != mdc.end ())
            addText (fieldName, it->second);
    }

    std::vector<iovec> iov;
    iov.reserve (fields.size ());
    for (auto & field : fields)
        iov.push_back ({field.data (), field.size ()});
    int result = sd_journal_sendv (iov.data (), static_cast<int> (iov.size ()));
    if (result < 0)
        errorHandler->error (LOG4CPLUS_TEXT ("JournalAppender: sd_journal_sendv failed: ")
            + helpers::convertIntegerToString (result));
}

} // namespace log4cplus

#endif // defined (LOG4CPLUS_WITH_SYSTEMD)
