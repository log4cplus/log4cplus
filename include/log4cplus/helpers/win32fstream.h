// -*- C++ -*-
//
//  Copyright (C) 2013-2014, 2026, Vaclav Haisman. All rights reserved.
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

#ifndef LOG4CPLUS_HELPERS_WIN32_FSTREAM_HEADER_
#define LOG4CPLUS_HELPERS_WIN32_FSTREAM_HEADER_

/**
 * @file win32fstream.h
 * @brief Bidirectional Win32 file streams with UTF-8 external storage.
 *
 * The stream classes in this header use `CreateFileW` and default to sharing
 * read, write, and delete access. This permits a file to be renamed while a
 * stream still has it open.
 */

#include <log4cplus/config.hxx>

#if defined (LOG4CPLUS_HAVE_PRAGMA_ONCE)
#pragma once
#endif

#if defined (_WIN32)

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ios>
#include <iostream>
#include <limits>
#include <locale>
#include <streambuf>
#include <string>
#include <system_error>
#include <vector>
#include <log4cplus/config/windowsh-inc.h>
#include <log4cplus/internal/utf8.h>

#if (__cplusplus >= 201703L) || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
#include <filesystem>
#define LOG4CPLUS_HELPERS_WIN32_FSTREAM_HAS_FILESYSTEM 1
#endif

/**
 * @brief UTF-8 Win32 stream classes and conversion policies.
 */
namespace log4cplus { namespace helpers {

/**
 * @brief Selects how malformed or unrepresentable text is handled.
 */
enum class conversion_error_policy {
    fail,   ///< Stop conversion and report `illegal_byte_sequence`.
    replace ///< Substitute a Unicode or code-page default character.
};

/**
 * @brief Additional Win32 and text-conversion options used while opening.
 *
 * The defaults allow other handles to read, write, rename, or delete the file
 * and reject lossy text conversion.
 */
struct win32_open_options {
    /// `CreateFileW` sharing flags.
    DWORD share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE
                       | FILE_SHARE_DELETE;
    /// Policy for invalid or unrepresentable text.
    conversion_error_policy conversion_errors = conversion_error_policy::fail;

    constexpr win32_open_options () noexcept = default;
};

namespace detail {

constexpr std::uint32_t unicode_byte_order_mark = 0xfeff;
constexpr std::uint32_t supplementary_code_point_first = 0x10000;
constexpr std::size_t default_byte_buffer_size = 8192;
constexpr std::size_t maximum_decoded_code_units = 8;
/**
 * @brief Windows code page identifier for 7-bit US-ASCII.
 * @see https://learn.microsoft.com/windows/win32/intl/code-page-identifiers
 */
constexpr unsigned int windows_us_ascii_code_page = 20127;

/** @brief Creates a system-category error from a Win32 error value. */
inline std::error_code windows_error (DWORD value = GetLastError ()) {
    return std::error_code (static_cast<int> (value), std::system_category ());
}

/** @brief Creates the standard error used for invalid text conversion. */
inline std::error_code text_error () {
    return std::make_error_code (std::errc::illegal_byte_sequence);
}

LOG4CPLUS_EXPORT unsigned int win32_acp_code_page ();
LOG4CPLUS_EXPORT unsigned int win32_utf8_code_page ();
LOG4CPLUS_EXPORT bool win32_is_utf8_code_page (unsigned int);
LOG4CPLUS_EXPORT std::size_t
win32_code_page_max_char_size (unsigned int, std::error_code &);
LOG4CPLUS_EXPORT bool win32_is_dbcs_lead_byte (unsigned int, unsigned char);
LOG4CPLUS_EXPORT bool win32_acp_path (char const *, std::wstring &,
                                      std::error_code &);
LOG4CPLUS_EXPORT bool win32_multibyte_to_wide (
    unsigned int, char const *, int, wchar_t *, int, int &, std::error_code &);
LOG4CPLUS_EXPORT bool win32_wide_to_multibyte (
    unsigned int, wchar_t const *, int, char *, int, bool, bool &, int &,
    std::error_code &);
LOG4CPLUS_EXPORT unsigned char
win32_code_page_default_char (unsigned int, std::error_code &);

/** @brief Tests whether a byte is a UTF-8 continuation code unit. */
constexpr bool is_utf8_continuation (unsigned char c) noexcept {
    return (c & 0xc0) == 0x80;
}

/** @brief Combines a UTF-16 surrogate pair into a Unicode scalar value. */
constexpr std::uint32_t surrogate_pair_to_scalar (std::uint32_t high,
                                                  std::uint32_t low) noexcept {
    // Reassemble the surrogate pair's two ten-bit payloads.
    return supplementary_code_point_first
           + ((high - high_surrogate_first) << 10)
           + (low - low_surrogate_first);
}

/** @brief Returns the length indicated by a valid UTF-8 leading byte. */
constexpr std::size_t utf8_sequence_length (unsigned char lead) noexcept {
    if (lead < 0x80) {
        return 1;
    }
    if (0xc2 <= lead && lead <= 0xdf) {
        return 2;
    }
    if (0xe0 <= lead && lead <= 0xef) {
        return 3;
    }
    if (0xf0 <= lead && lead <= 0xf4) {
        return 4;
    }
    return 0;
}

/** @brief Decodes and validates one complete UTF-8 sequence. */
constexpr bool decode_utf8 (unsigned char const * p, std::size_t n,
                            std::uint32_t & cp, std::size_t & used) noexcept {
    if (!n) {
        return false;
    }
    unsigned char const b = p[0];
    std::size_t const need = utf8_sequence_length (b);
    if (!need || n < need) {
        return false;
    }
    // Strip the UTF-8 prefix from the leading byte to seed the scalar value.
    cp = b & (need == 1 ? 0x7f : need == 2 ? 0x1f : need == 3 ? 0x0f : 0x07);
    for (std::size_t i = 1; i != need; ++i) {
        if ((p[i] & 0xc0) != 0x80) {
            return false;
        }
        // Append the continuation byte's six payload bits.
        cp = (cp << 6) | (p[i] & 0x3f);
    }
    // Reject overlong UTF-8 encodings, surrogates, and values above U+10FFFF.
    if ((need == 2 && cp < 0x80) || (need == 3 && cp < 0x800)
        || (need == 4 && cp < supplementary_code_point_first)
        || !is_scalar (cp)) {
        return false;
    }
    used = need;
    return true;
}

/** @brief Converts an ACP-encoded path to UTF-16 for Win32. */
inline std::wstring acp_path (std::error_code & ec, char const * path) {
    std::wstring result;
    if (!win32_acp_path (path, result, ec)) {
        return std::wstring ();
    }
    return result;
}

/** @brief Selects the Windows code page represented by a C++ locale. */
inline unsigned int locale_code_page (std::locale const & loc) {
    std::string const name = loc.name ();
    if (name == "C" || name == "POSIX") {
        return windows_us_ascii_code_page;
    }
    std::string::size_type const dot = name.rfind ('.');
    if (dot != std::string::npos) {
        std::string const suffix = name.substr (dot + 1);
        std::string lower = suffix;
        std::transform (lower.begin (), lower.end (), lower.begin (),
                        ::tolower);
        if (lower == "utf8" || lower == "utf-8") {
            return win32_utf8_code_page ();
        }
        char * end = nullptr;
        unsigned long const cp = std::strtoul (suffix.c_str (), &end, 10);
        if (end && !*end && cp) {
            return static_cast<unsigned int> (cp);
        }
    }
    return win32_acp_code_page ();
}

/** @brief Encodes a Unicode scalar value as one or two UTF-16 code units. */
template <typename CharT>
constexpr std::size_t scalar_to_utf16 (CharT * out, std::uint32_t cp) noexcept {
    if (cp <= 0xffff) {
        out[0] = static_cast<CharT> (cp);
        return 1;
    }

    // Split a supplementary scalar's 20 payload bits into a surrogate pair.
    std::uint32_t const payload = cp - supplementary_code_point_first;
    out[0] = static_cast<CharT> (high_surrogate_first + (payload >> 10));
    out[1] = static_cast<CharT> (low_surrogate_first + (payload & 0x3ff));
    return 2;
}

} // namespace detail

/**
 * @brief Converts between stream characters and Unicode scalar values.
 *
 * `basic_win32_filebuf` uses this class as its codec customization point. A
 * custom codec must provide the same `imbue`, `encode`, `finish`, and `decode`
 * operations as the supplied specializations.
 *
 * @tparam CharT Character type presented by the stream.
 */
template <typename CharT> class utf8_codec;

/**
 * @brief Codec for locale-encoded narrow characters.
 *
 * The code page is derived from the stream locale. UTF-8 locales are validated
 * directly; Windows code pages use Win32 conversion helpers.
 */
template <> class utf8_codec<char> {
  public:
    utf8_codec ()
        : cp_ (0), max_char_size_ (0), code_page_error_ (), expected_ (0) {
        set_code_page (detail::locale_code_page (std::locale ()));
    }

    /** @brief Resets conversion state for the supplied locale. */
    void imbue (std::locale const & loc) {
        set_code_page (detail::locale_code_page (loc));
        pending_.clear ();
        expected_ = 0;
    }

    /** @brief Encodes one narrow code unit into complete Unicode scalars. */
    bool encode (char ch, std::vector<std::uint32_t> & out,
                 conversion_error_policy policy, std::error_code & ec) {
        pending_.push_back (static_cast<unsigned char> (ch));
        if (detail::win32_is_utf8_code_page (cp_)) {
            if (pending_.size () == 1) {
                unsigned char const b = pending_[0];
                expected_ = detail::utf8_sequence_length (b);
            }
            if (expected_ && pending_.size () < expected_) {
                return true;
            }
            std::uint32_t cp;
            std::size_t used = 0;
            if (expected_
                && detail::decode_utf8 (&pending_[0], pending_.size (), cp,
                                        used)) {
                out.push_back (cp);
            } else if (policy == conversion_error_policy::replace) {
                out.push_back (detail::unicode_replacement_character);
            } else {
                ec = detail::text_error ();
                return false;
            }
            pending_.clear ();
            expected_ = 0;
            return true;
        }
        if (code_page_error_) {
            ec = code_page_error_;
            return false;
        }
        if (pending_.size () == 1 && max_char_size_ > 1
            && detail::win32_is_dbcs_lead_byte (cp_, pending_[0])) {
            return true;
        }
        wchar_t w[2] = {};
        int n = 0;
        std::error_code ignored_error;
        if (!detail::win32_multibyte_to_wide (
                cp_, reinterpret_cast<char const *> (&pending_[0]),
                static_cast<int> (pending_.size ()), w, 2, n,
                ignored_error)) {
            if (pending_.size () < max_char_size_) {
                return true;
            }
            if (policy == conversion_error_policy::replace) {
                out.push_back (detail::unicode_replacement_character);
            } else {
                ec = detail::text_error ();
                return false;
            }
        } else if (n == 2 && detail::is_high_surrogate (w[0])
                   && detail::is_low_surrogate (w[1])) {
            out.push_back (detail::surrogate_pair_to_scalar (w[0], w[1]));
        } else {
            out.push_back (static_cast<std::uint32_t> (w[0]));
        }
        pending_.clear ();
        return true;
    }

    /** @brief Resolves an incomplete narrow sequence at end of input. */
    bool finish (std::vector<std::uint32_t> & out,
                 conversion_error_policy policy, std::error_code & ec) {
        if (pending_.empty ()) {
            return true;
        }
        if (policy == conversion_error_policy::replace) {
            out.push_back (detail::unicode_replacement_character);
            pending_.clear ();
            return true;
        }
        ec = detail::text_error ();
        return false;
    }

    /** @brief Converts one Unicode scalar to the active narrow code page. */
    bool decode (std::uint32_t cp, char * out, std::size_t & n,
                 conversion_error_policy policy, std::error_code & ec) {
        wchar_t w[2];
        std::size_t const wn = detail::scalar_to_utf16 (w, cp);
        bool used_default = false;
        int count = 0;
        if (!detail::win32_wide_to_multibyte (
                cp_, w, static_cast<int> (wn), out,
                static_cast<int> (detail::maximum_decoded_code_units), true,
                used_default, count, ec)) {
            return false;
        }
        if (used_default && policy == conversion_error_policy::fail) {
            ec = detail::text_error ();
            return false;
        }
        n = static_cast<std::size_t> (count);
        return true;
    }

  private:
    /** @brief Caches conversion metadata for a Windows code page. */
    void set_code_page (unsigned int code_page) {
        cp_ = code_page;
        max_char_size_ =
            detail::win32_code_page_max_char_size (cp_, code_page_error_);
    }

    unsigned int cp_;
    std::size_t max_char_size_;
    std::error_code code_page_error_;
    std::vector<unsigned char> pending_;
    std::size_t expected_;
};

/**
 * @brief Shared UTF-16 codec for 16-bit stream character types.
 *
 * Surrogate pairs may span calls to `encode()`. Unpaired surrogates either
 * fail conversion or become U+FFFD according to the selected error policy.
 *
 * @tparam CharT A 16-bit character type.
 */
template <typename CharT> class utf16_codec {
  public:
    utf16_codec () : high_ (0) {
    }

    /** @brief Resets pending UTF-16 conversion state. */
    void imbue (std::locale const &) {
        high_ = 0;
    }

    /** @brief Encodes one UTF-16 code unit into complete Unicode scalars. */
    bool encode (CharT ch, std::vector<std::uint32_t> & out,
                 conversion_error_policy policy, std::error_code & ec) {
        std::uint32_t const c = static_cast<std::uint32_t> (ch);
        if (high_) {
            if (detail::is_low_surrogate (c)) {
                out.push_back (detail::surrogate_pair_to_scalar (high_, c));
            } else if (policy == conversion_error_policy::replace) {
                out.push_back (detail::unicode_replacement_character);
                high_ = 0;
                return encode (ch, out, policy, ec);
            } else {
                ec = detail::text_error ();
                return false;
            }
            high_ = 0;
            return true;
        }
        if (detail::is_high_surrogate (c)) {
            high_ = c;
            return true;
        }
        if (detail::is_low_surrogate (c)) {
            if (policy == conversion_error_policy::replace) {
                out.push_back (detail::unicode_replacement_character);
            } else {
                ec = detail::text_error ();
                return false;
            }
        } else {
            out.push_back (c);
        }
        return true;
    }

    /** @brief Resolves a pending high surrogate at end of input. */
    bool finish (std::vector<std::uint32_t> & out,
                 conversion_error_policy policy, std::error_code & ec) {
        if (!high_) {
            return true;
        }
        high_ = 0;
        if (policy == conversion_error_policy::replace) {
            out.push_back (detail::unicode_replacement_character);
            return true;
        }
        ec = detail::text_error ();
        return false;
    }

    /** @brief Converts one Unicode scalar to UTF-16 code units. */
    bool decode (std::uint32_t cp, CharT * out, std::size_t & n,
                 conversion_error_policy, std::error_code &) {
        n = detail::scalar_to_utf16 (out, cp);
        return true;
    }

  private:
    std::uint32_t high_;
};

/** @brief UTF-16 codec for Windows `wchar_t`. */
template <> class utf8_codec<wchar_t> : public utf16_codec<wchar_t> {};

/** @brief UTF-16 codec for `char16_t`. */
template <> class utf8_codec<char16_t> : public utf16_codec<char16_t> {};

/** @brief Unicode scalar-value codec for `char32_t`. */
template <> class utf8_codec<char32_t> {
  public:
    /** @brief Accepts a locale without changing scalar conversion. */
    void imbue (std::locale const &) {
    }

    /** @brief Validates and emits one UTF-32 scalar value. */
    bool encode (char32_t ch, std::vector<std::uint32_t> & out,
                 conversion_error_policy p, std::error_code & ec) {
        std::uint32_t const c = static_cast<std::uint32_t> (ch);
        if (detail::is_scalar (c)) {
            out.push_back (c);
        } else if (p == conversion_error_policy::replace) {
            out.push_back (detail::unicode_replacement_character);
        } else {
            ec = detail::text_error ();
            return false;
        }
        return true;
    }

    /** @brief Completes the stateless UTF-32 conversion. */
    bool finish (std::vector<std::uint32_t> &, conversion_error_policy,
                 std::error_code &) {
        return true;
    }

    /** @brief Converts one Unicode scalar to a UTF-32 code unit. */
    bool decode (std::uint32_t cp, char32_t * out, std::size_t & n,
                 conversion_error_policy, std::error_code &) {
        out[0] = static_cast<char32_t> (cp);
        n = 1;
        return true;
    }
};

#if defined(__cpp_char8_t)
/** @brief Validating UTF-8 codec for C++20 `char8_t`. */
template <> class utf8_codec<char8_t> {
  public:
    utf8_codec () : expected_ (0) {
    }

    /** @brief Resets pending UTF-8 conversion state. */
    void imbue (std::locale const &) {
        pending_.clear ();
        expected_ = 0;
    }

    /** @brief Encodes and validates one UTF-8 code unit. */
    bool encode (char8_t c, std::vector<std::uint32_t> & out,
                 conversion_error_policy policy, std::error_code & ec) {
        pending_.push_back (static_cast<unsigned char> (c));
        if (pending_.size () == 1) {
            unsigned char const b = pending_[0];
            expected_ = detail::utf8_sequence_length (b);
        }
        if (expected_ && pending_.size () < expected_) {
            return true;
        }
        std::uint32_t cp;
        std::size_t used = 0;
        if (expected_
            && detail::decode_utf8 (&pending_[0], pending_.size (), cp, used)) {
            out.push_back (cp);
        } else if (policy == conversion_error_policy::replace) {
            out.push_back (detail::unicode_replacement_character);
        } else {
            ec = detail::text_error ();
            return false;
        }
        pending_.clear ();
        expected_ = 0;
        return true;
    }

    /** @brief Resolves an incomplete UTF-8 sequence at end of input. */
    bool finish (std::vector<std::uint32_t> & out,
                 conversion_error_policy policy, std::error_code & ec) {
        if (pending_.empty ()) {
            return true;
        }
        if (policy == conversion_error_policy::replace) {
            out.push_back (detail::unicode_replacement_character);
            pending_.clear ();
            return true;
        }
        ec = detail::text_error ();
        return false;
    }

    /** @brief Converts one Unicode scalar to UTF-8 code units. */
    bool decode (std::uint32_t cp, char8_t * out, std::size_t & n,
                 conversion_error_policy, std::error_code &) {
        char b[detail::maximum_utf8_sequence_length];
        n = detail::encode_utf8 (b, cp);
        for (std::size_t i = 0; i < n; ++i) {
            out[i] = static_cast<char8_t> (b[i]);
        }
        return true;
    }

  private:
    std::vector<unsigned char> pending_;
    std::size_t expected_;
};
#endif

template <typename CharT, typename Traits = std::char_traits<CharT>,
          typename Codec = utf8_codec<CharT>>
/**
 * @brief Win32-backed, bidirectional stream buffer with UTF-8 file storage.
 *
 * ## File behavior
 *
 * - Uses `CreateFileW` for every path.
 * - Maintains an explicit 64-bit byte position.
 * - Implements `app` writes atomically at the current end of file.
 * - Converts LF and CRLF in text mode and consumes one leading UTF-8 BOM.
 * - Keeps UTF-8 encoding in binary mode, while disabling newline and BOM
 *   handling.
 *
 * Seek offsets are external UTF-8 byte counts. Targets on continuation bytes
 * advance to the next syntactic sequence boundary. Traversed continuation
 * bytes are not decoded, while a malformed leading byte at the adjusted
 * position remains visible to the configured conversion policy. Targets
 * beyond EOF are preserved, while `app` writes continue to target EOF. Output
 * opens request read access so every mode can inspect seek targets. No
 * position history is retained.
 *
 * @tparam CharT Character type exposed by the stream buffer.
 * @tparam Traits Character traits for `CharT`.
 * @tparam Codec Converter between `CharT` and Unicode scalar values.
 */
class basic_win32_filebuf : public std::basic_streambuf<CharT, Traits> {
  public:
    using char_type = CharT;
    using traits_type = Traits;
    using int_type = typename Traits::int_type;
    using pos_type = typename Traits::pos_type;
    using off_type = typename Traits::off_type;
    using openmode = std::ios_base::openmode;
    using native_handle_type = HANDLE;

    basic_win32_filebuf ()
        : handle_ (INVALID_HANDLE_VALUE), mode_ (openmode (0)), options_ (),
          offset_ (0), get_begin_ (0), get_end_ (0), direction_ (none),
          conversion_started_ (false), get_count_ (0), has_putback_ (false),
          putback_position_safe_ (false), putback_begin_ (0),
          owned_byte_buffer_ (detail::default_byte_buffer_size),
          byte_buffer_ (&owned_byte_buffer_[0]),
          byte_capacity_ (owned_byte_buffer_.size ()), byte_size_ (0),
          read_buffer_begin_ (0), read_buffer_size_ (0) {
        codec_.imbue (this->getloc ());
        this->setg (nullptr, nullptr, nullptr);
    }

    ~basic_win32_filebuf () override {
        (void) close ();
    }

    basic_win32_filebuf (basic_win32_filebuf const &) = delete;
    basic_win32_filebuf & operator= (basic_win32_filebuf const &) = delete;

    [[nodiscard]]
    basic_win32_filebuf *
    open (wchar_t const * path, openmode mode,
          win32_open_options const & opts = win32_open_options ()) {
        if (is_open () || !path) {
            last_error_ = std::make_error_code (std::errc::invalid_argument);
            return nullptr;
        }
        if (!(mode & (std::ios_base::in | std::ios_base::out))
            && (mode & (std::ios_base::app | std::ios_base::trunc))) {
            mode |= std::ios_base::out;
        }
        DWORD access = 0;
        if (mode & (std::ios_base::in | std::ios_base::out)) {
            access |= GENERIC_READ;
        }
        if (mode & std::ios_base::out) {
            access |=
                (mode & std::ios_base::app) ? FILE_APPEND_DATA : GENERIC_WRITE;
        }
        if (!access) {
            last_error_ = std::make_error_code (std::errc::invalid_argument);
            return nullptr;
        }
        DWORD disposition = OPEN_EXISTING;
        if (mode & std::ios_base::trunc) {
            disposition = CREATE_ALWAYS;
        } else if (mode & std::ios_base::app) {
            disposition = OPEN_ALWAYS;
        } else if ((mode & std::ios_base::out) && !(mode & std::ios_base::in)) {
            disposition = CREATE_ALWAYS;
        }
#if defined(__cpp_lib_ios_noreplace)
        if (mode & std::ios_base::noreplace) {
            disposition = CREATE_NEW;
        }
#endif
        HANDLE const h =
            CreateFileW (path, access, opts.share_mode, nullptr, disposition,
                         FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE) {
            last_error_ = detail::windows_error ();
            return nullptr;
        }
        handle_ = h;
        mode_ = mode;
        options_ = opts;
        offset_ = 0;
        clear_byte_buffer ();
        clear_get_area ();
        last_error_.clear ();
        direction_ = none;
        conversion_started_ = false;
        codec_.imbue (this->getloc ());
        if (mode & (std::ios_base::ate | std::ios_base::app)) {
            if (!file_size (offset_)) {
                CloseHandle (handle_);
                handle_ = INVALID_HANDLE_VALUE;
                mode_ = openmode (0);
                return nullptr;
            }
        }
        return this;
    }

    [[nodiscard]]
    basic_win32_filebuf *
    open (char const * path, openmode mode,
          win32_open_options const & opts = win32_open_options ()) {
        std::error_code ec;
        std::wstring const wide = detail::acp_path (ec, path);
        if (ec) {
            last_error_ = ec;
            return nullptr;
        }
        return open (wide.c_str (), mode, opts);
    }

    [[nodiscard]]
    basic_win32_filebuf *
    open (std::wstring const & p, openmode m,
          win32_open_options const & o = win32_open_options ()) {
        return open (p.c_str (), m, o);
    }

    [[nodiscard]]
    basic_win32_filebuf *
    open (std::string const & p, openmode m,
          win32_open_options const & o = win32_open_options ()) {
        return open (p.c_str (), m, o);
    }
#if defined(LOG4CPLUS_HELPERS_WIN32_FSTREAM_HAS_FILESYSTEM)
    [[nodiscard]]
    basic_win32_filebuf *
    open (std::filesystem::path const & p, openmode m,
          win32_open_options const & o = win32_open_options ()) {
        return open (p.c_str (), m, o);
    }
#endif
    [[nodiscard]]
    basic_win32_filebuf * close () {
        if (!is_open ()) {
            return nullptr;
        }
        bool ok = sync () == 0;
        if (!CloseHandle (handle_)) {
            last_error_ = detail::windows_error ();
            ok = false;
        }
        handle_ = INVALID_HANDLE_VALUE;
        mode_ = openmode (0);
        direction_ = none;
        clear_byte_buffer ();
        clear_get_area ();
        return ok ? this : nullptr;
    }

    [[nodiscard]]
    bool is_open () const {
        return handle_ != INVALID_HANDLE_VALUE;
    }

    [[nodiscard]]
    native_handle_type native_handle () const {
        return handle_;
    }

    [[nodiscard]]
    std::error_code last_error () const {
        return last_error_;
    }

  protected:
    /**
     * @brief Selects caller-owned byte-buffer storage before opening a file.
     *
     * `n` counts `CharT` elements, so the raw capacity is
     * `n * sizeof(CharT)`. The caller must keep non-null storage alive and
     * unchanged while this buffer uses it. Passing `(nullptr, 0)` selects the
     * minimally buffered four-byte internal mode.
     */
    basic_win32_filebuf * setbuf (char_type * s, std::streamsize n) override {
        if (is_open ()) {
            last_error_ = std::make_error_code (std::errc::invalid_argument);
            return nullptr;
        }

        if (!s && n == 0) {
            owned_byte_buffer_.assign (detail::maximum_utf8_sequence_length, 0);
            byte_buffer_ = &owned_byte_buffer_[0];
            byte_capacity_ = owned_byte_buffer_.size ();
            clear_byte_buffer ();
            last_error_.clear ();
            return this;
        }

        if (!s || n <= 0
            || static_cast<std::uintmax_t> (n)
                   > (std::numeric_limits<std::size_t>::max) ()
                         / sizeof (char_type)) {
            last_error_ = std::make_error_code (std::errc::invalid_argument);
            return nullptr;
        }

        std::size_t const capacity =
            static_cast<std::size_t> (n) * sizeof (char_type);
        if (capacity < detail::maximum_utf8_sequence_length) {
            last_error_ = std::make_error_code (std::errc::invalid_argument);
            return nullptr;
        }

        byte_buffer_ = reinterpret_cast<char *> (s);
        byte_capacity_ = capacity;
        clear_byte_buffer ();
        last_error_.clear ();
        return this;
    }

    int_type overflow (int_type ch = Traits::eof ()) override {
        if (!(mode_ & std::ios_base::out) || !is_open ()) {
            return Traits::eof ();
        }
        if (!prepare_write ()) {
            return Traits::eof ();
        }
        if (!Traits::eq_int_type (ch, Traits::eof ())) {
            std::vector<std::uint32_t> cps;
            if (!codec_.encode (Traits::to_char_type (ch), cps,
                                options_.conversion_errors, last_error_)) {
                return Traits::eof ();
            }
            conversion_started_ = true;
            if (!emit (cps)) {
                return Traits::eof ();
            }
        }
        return Traits::not_eof (ch);
    }

    std::streamsize xsputn (char_type const * s, std::streamsize n) override {
        std::streamsize done = 0;
        for (; done < n; ++done) {
            if (Traits::eq_int_type (overflow (Traits::to_int_type (s[done])),
                                     Traits::eof ())) {
                break;
            }
        }
        return done;
    }

    int sync () override {
        if (direction_ == writing) {
            std::vector<std::uint32_t> cps;
            if (!codec_.finish (cps, options_.conversion_errors, last_error_)
                || !emit (cps) || !flush_bytes ()) {
                return -1;
            }
        }
        return 0;
    }

    int_type underflow () override {
        if (!(mode_ & std::ios_base::in) || !is_open ()) {
            return Traits::eof ();
        }
        if (this->gptr () && this->gptr () < this->egptr ()) {
            return Traits::to_int_type (*this->gptr ());
        }
        if (!prepare_read ()) {
            return Traits::eof ();
        }

        bool const preserve =
            this->gptr () && this->eback () && this->gptr () > this->eback ();
        char_type previous = char_type ();
        bool previous_position_safe = false;
        std::uint64_t previous_begin = 0;
        if (preserve) {
            previous = this->gptr ()[-1];
            if (get_count_ == 1) {
                previous_position_safe = true;
                previous_begin = get_begin_;
            } else if (get_count_ == 0 && has_putback_) {
                previous_position_safe = putback_position_safe_;
                previous_begin = putback_begin_;
            }
        }

        std::uint32_t cp;
        std::uint64_t begin = offset_, end = offset_;
        if (!read_scalar (cp, end)) {
            install_get_area (preserve, previous, previous_position_safe,
                              previous_begin, 0);
            return Traits::eof ();
        }
        if (!(mode_ & std::ios_base::binary) && begin == 0
            && cp == detail::unicode_byte_order_mark) {
            begin = end;
            if (!read_scalar (cp, end)) {
                install_get_area (preserve, previous, previous_position_safe,
                                  previous_begin, 0);
                return Traits::eof ();
            }
        }
        if (!(mode_ & std::ios_base::binary) && cp == '\r') {
            std::uint32_t next;
            std::uint64_t peek = end;
            if (read_scalar (next, peek) && next == '\n') {
                cp = '\n';
                end = peek;
            }
        }
        std::size_t count = 0;
        if (!codec_.decode (cp, get_buf_ + 1, count, options_.conversion_errors,
                            last_error_)) {
            install_get_area (preserve, previous, previous_position_safe,
                              previous_begin, 0);
            return Traits::eof ();
        }
        get_begin_ = begin;
        get_end_ = end;
        offset_ = end;
        conversion_started_ = true;
        install_get_area (preserve, previous, previous_position_safe,
                          previous_begin, count);
        return Traits::to_int_type (*this->gptr ());
    }

    pos_type seekoff (off_type off, std::ios_base::seekdir way,
                      openmode = std::ios_base::in
                                 | std::ios_base::out) override {
        if (sync () != 0) {
            return bad_pos ();
        }
        std::uint64_t base = 0;
        if (way == std::ios_base::beg) {
            base = 0;
        } else if (way == std::ios_base::end) {
            if (!file_size (base)) {
                return bad_pos ();
            }
        } else if (way == std::ios_base::cur) {
            if (!current_position (base)) {
                return bad_pos ();
            }
            if (off == 0) {
                return position_representable (base)
                           ? pos_type (static_cast<off_type> (base))
                           : bad_pos ();
            }
        } else {
            return bad_pos ();
        }

        std::uint64_t target = 0;
        if (!add_offset (target, base, off)
            || !resynchronize_position (target)) {
            return bad_pos ();
        }
        reset_position (target);
        return pos_type (static_cast<off_type> (target));
    }

    pos_type seekpos (pos_type p, openmode = std::ios_base::in
                                             | std::ios_base::out) override {
        if (sync () != 0) {
            return bad_pos ();
        }
        off_type const o = static_cast<off_type> (p);
        if (o < 0
            || static_cast<std::uintmax_t> (o)
                   > (std::numeric_limits<std::uint64_t>::max) ()) {
            return bad_pos ();
        }
        std::uint64_t target = static_cast<std::uint64_t> (o);
        if (!resynchronize_position (target)) {
            return bad_pos ();
        }
        reset_position (target);
        return pos_type (static_cast<off_type> (target));
    }

    void imbue (std::locale const & loc) override {
        if (conversion_started_) {
            throw std::ios_base::failure ("encoding cannot change after I/O",
                                          detail::text_error ());
        }
        codec_.imbue (loc);
    }

  private:
    enum direction { none, reading, writing };

    /** @brief Returns the stream position used to report seek failure. */
    [[nodiscard]]
    static pos_type bad_pos () {
        return pos_type (off_type (-1));
    }

    /** @brief Tests whether a byte offset fits in the stream position type. */
    [[nodiscard]]
    static constexpr bool
    position_representable (std::uint64_t position) noexcept {
        return static_cast<std::uintmax_t> (position)
               <= static_cast<std::uintmax_t> (
                   (std::numeric_limits<off_type>::max) ());
    }

    /** @brief Adds a signed byte displacement without wrapping. */
    [[nodiscard]]
    static constexpr bool
    add_offset (std::uint64_t & target, std::uint64_t base,
                off_type displacement) noexcept {
        if (displacement >= 0) {
            std::uintmax_t const amount =
                static_cast<std::uintmax_t> (displacement);
            if (amount > (std::numeric_limits<std::uint64_t>::max) ()
                || base > (std::numeric_limits<std::uint64_t>::max) ()
                              - static_cast<std::uint64_t> (amount)) {
                return false;
            }
            target = base + static_cast<std::uint64_t> (amount);
        } else {
            std::uintmax_t const amount =
                static_cast<std::uintmax_t> (-(displacement + 1)) + 1;
            if (amount > base) {
                return false;
            }
            target = base - static_cast<std::uint64_t> (amount);
        }
        return position_representable (target);
    }

    /** @brief Advances a byte target to a syntactic UTF-8 boundary. */
    [[nodiscard]]
    bool resynchronize_position (std::uint64_t & target) {
        if (target == 0) {
            return true;
        }

        std::uint64_t size = 0;
        if (!file_size (size)) {
            return false;
        }
        if (target >= size) {
            return position_representable (target);
        }

        unsigned char byte = 0;
        while (target < size) {
            if (!read_byte (byte, target)) {
                std::uint64_t updated_size = 0;
                return file_size (updated_size) && target >= updated_size;
            }
            if (!detail::is_utf8_continuation (byte)) {
                return position_representable (target);
            }
            ++target;
        }
        return position_representable (target);
    }

    /** @brief Clears decoded input and its one-code-unit putback area. */
    void clear_get_area () {
        get_count_ = 0;
        has_putback_ = false;
        putback_position_safe_ = false;
        putback_begin_ = 0;
        this->setg (nullptr, nullptr, nullptr);
    }

    /** @brief Installs decoded input with an optional putback code unit. */
    void install_get_area (bool preserve, char_type previous,
                           bool previous_position_safe,
                           std::uint64_t previous_begin, std::size_t count) {
        char_type * const current = get_buf_ + 1;
        if (preserve) {
            get_buf_[0] = previous;
            has_putback_ = true;
            putback_position_safe_ = previous_position_safe;
            putback_begin_ = previous_begin;
            this->setg (get_buf_, current, current + count);
        } else {
            has_putback_ = false;
            putback_position_safe_ = false;
            putback_begin_ = 0;
            this->setg (current, current, current + count);
        }
        get_count_ = count;
    }

    /** @brief Returns the safe external offset represented by the get area. */
    [[nodiscard]]
    bool current_position (std::uint64_t & position) const {
        if (!this->gptr ()) {
            position = offset_;
            return true;
        }
        if (this->gptr () == this->egptr ()) {
            position = offset_;
            return true;
        }
        char_type const * const current = get_buf_ + 1;
        if (this->gptr () == current) {
            position = get_begin_;
            return true;
        }
        if (has_putback_ && this->gptr () == this->eback ()
            && putback_position_safe_) {
            position = putback_begin_;
            return true;
        }
        return false;
    }

    /** @brief Clears pending output and cached input byte ranges. */
    void clear_byte_buffer () {
        byte_size_ = 0;
        read_buffer_begin_ = 0;
        read_buffer_size_ = 0;
    }

    /** @brief Resets buffering and conversion state at a byte offset. */
    void reset_position (std::uint64_t p) {
        offset_ = p;
        direction_ = none;
        clear_byte_buffer ();
        codec_.imbue (this->getloc ());
        clear_get_area ();
    }

    /** @brief Changes direction to output while preserving a safe cursor. */
    [[nodiscard]]
    bool prepare_write () {
        if (direction_ == reading) {
            std::uint64_t position = 0;
            if (!current_position (position)) {
                last_error_ = detail::text_error ();
                return false;
            }
            offset_ = position;
            clear_byte_buffer ();
            clear_get_area ();
            codec_.imbue (this->getloc ());
        }
        direction_ = writing;
        return true;
    }

    /** @brief Changes direction to input after flushing pending output. */
    [[nodiscard]]
    bool prepare_read () {
        if (direction_ == writing && sync () != 0) {
            return false;
        }
        if (direction_ == writing) {
            clear_byte_buffer ();
        }
        direction_ = reading;
        return true;
    }

    /** @brief Encodes Unicode scalars into the pending UTF-8 byte buffer. */
    [[nodiscard]]
    bool emit (std::vector<std::uint32_t> const & cps) {
        for (std::size_t i = 0; i < cps.size (); ++i) {
            if (!(mode_ & std::ios_base::binary) && cps[i] == '\n') {
                char const carriage_return = '\r';
                if (!queue_bytes (&carriage_return, 1)) {
                    return false;
                }
            }
            char encoded[detail::maximum_utf8_sequence_length];
            std::size_t const count = detail::encode_utf8 (encoded, cps[i]);
            if (!queue_bytes (encoded, count)) {
                return false;
            }
        }
        return true;
    }

    /** @brief Copies bytes into the bounded output buffer, flushing as needed.
     */
    [[nodiscard]]
    bool queue_bytes (char const * bytes, std::size_t count) {
        std::size_t copied = 0;
        while (copied != count) {
            if (byte_size_ == byte_capacity_ && !flush_bytes ()) {
                return false;
            }
            std::size_t const available = byte_capacity_ - byte_size_;
            std::size_t const amount = (std::min) (available, count - copied);
            std::memcpy (byte_buffer_ + byte_size_, bytes + copied, amount);
            byte_size_ += amount;
            copied += amount;
        }
        return true;
    }

    /** @brief Writes all pending UTF-8 bytes using positional Win32 I/O. */
    [[nodiscard]]
    bool flush_bytes () {
        std::size_t done = 0;
        while (done < byte_size_) {
            DWORD const chunk = static_cast<DWORD> ((std::min) (
                byte_size_ - done, static_cast<std::size_t> (
                                       (std::numeric_limits<DWORD>::max) ())));
            OVERLAPPED ov = {};
            std::uint64_t const at = offset_;
            if (mode_ & std::ios_base::app) {
                ov.Offset = 0xffffffffu;
                ov.OffsetHigh = 0xffffffffu;
            } else {
                ov.Offset = static_cast<DWORD> (at);
                ov.OffsetHigh = static_cast<DWORD> (at >> 32);
            }
            DWORD written = 0;
            if (!WriteFile (handle_, byte_buffer_ + done, chunk, &written,
                            &ov)) {
                last_error_ = detail::windows_error ();
                return false;
            }
            if (!written) {
                last_error_ = std::make_error_code (std::errc::io_error);
                return false;
            }
            done += written;
            if (mode_ & std::ios_base::app) {
                if (!file_size (offset_)) {
                    return false;
                }
            } else {
                offset_ += written;
            }
        }
        byte_size_ = 0;
        return true;
    }

    /** @brief Refills the raw input cache from an explicit file offset. */
    [[nodiscard]]
    bool fill_read_buffer (std::uint64_t at) {
        OVERLAPPED ov = {};
        ov.Offset = static_cast<DWORD> (at);
        ov.OffsetHigh = static_cast<DWORD> (at >> 32);
        DWORD const request = static_cast<DWORD> ((std::min) (
            byte_capacity_,
            static_cast<std::size_t> ((std::numeric_limits<DWORD>::max) ())));
        DWORD got = 0;
        if (!ReadFile (handle_, byte_buffer_, request, &got, &ov)) {
            last_error_ = detail::windows_error ();
            read_buffer_size_ = 0;
            return false;
        }
        read_buffer_begin_ = at;
        read_buffer_size_ = got;
        return got != 0;
    }

    /** @brief Reads one byte from the cached positional input window. */
    [[nodiscard]]
    bool read_byte (unsigned char & b, std::uint64_t at) {
        if (at < read_buffer_begin_
            || at - read_buffer_begin_ >= read_buffer_size_) {
            if (!fill_read_buffer (at)) {
                return false;
            }
        }
        b = static_cast<unsigned char> (
            byte_buffer_[static_cast<std::size_t> (at - read_buffer_begin_)]);
        return true;
    }

    /** @brief Reads and validates one UTF-8 scalar at a byte offset. */
    [[nodiscard]]
    bool read_scalar (std::uint32_t & cp, std::uint64_t & at) {
        unsigned char b[4];
        if (!read_byte (b[0], at)) {
            return false;
        }
        std::size_t const need = detail::utf8_sequence_length (b[0]);
        if (!need) {
            if (options_.conversion_errors
                == conversion_error_policy::replace) {
                cp = detail::unicode_replacement_character;
                ++at;
                return true;
            }
            last_error_ = detail::text_error ();
            return false;
        }
        for (std::size_t i = 1; i < need; ++i) {
            if (!read_byte (b[i], at + i)) {
                if (options_.conversion_errors
                    == conversion_error_policy::replace) {
                    cp = detail::unicode_replacement_character;
                    ++at;
                    return true;
                }
                last_error_ = detail::text_error ();
                return false;
            }
        }
        std::size_t used = 0;
        if (!detail::decode_utf8 (b, need, cp, used)) {
            if (options_.conversion_errors
                == conversion_error_policy::replace) {
                cp = detail::unicode_replacement_character;
                ++at;
                return true;
            }
            last_error_ = detail::text_error ();
            return false;
        }
        at += used;
        return true;
    }

    /** @brief Queries the current 64-bit file size. */
    [[nodiscard]]
    bool file_size (std::uint64_t & size) {
        LARGE_INTEGER s = {};
        if (!GetFileSizeEx (handle_, &s)) {
            last_error_ = detail::windows_error ();
            return false;
        }
        size = static_cast<std::uint64_t> (s.QuadPart);
        return true;
    }

    HANDLE handle_;
    openmode mode_;
    win32_open_options options_;
    Codec codec_;
    std::uint64_t offset_, get_begin_, get_end_;
    direction direction_;
    bool conversion_started_;
    char_type get_buf_[detail::maximum_decoded_code_units + 1];
    std::size_t get_count_;
    bool has_putback_;
    bool putback_position_safe_;
    std::uint64_t putback_begin_;
    std::vector<char> owned_byte_buffer_;
    char * byte_buffer_;
    std::size_t byte_capacity_;
    std::size_t byte_size_;
    std::uint64_t read_buffer_begin_;
    std::size_t read_buffer_size_;
    std::error_code last_error_;
};

namespace detail {
template <typename CharT, typename Traits, typename Codec>
struct streambuf_holder {
    basic_win32_filebuf<CharT, Traits, Codec> buf;
};

template <typename CharT, typename Traits, typename Codec, typename StreamT>
class basic_win32_stream_base
    : private streambuf_holder<CharT, Traits, Codec>,
      public StreamT {
    using holder = streambuf_holder<CharT, Traits, Codec>;

  public:
    using filebuf_type = basic_win32_filebuf<CharT, Traits, Codec>;
    using openmode = std::ios_base::openmode;

    basic_win32_stream_base () : holder (), StreamT (&this->buf) {
    }

    filebuf_type * rdbuf () const {
        return const_cast<filebuf_type *> (&this->buf);
    }

    [[nodiscard]]
    bool is_open () const {
        return this->buf.is_open ();
    }

    void close () {
        if (!this->buf.close ()) {
            this->setstate (std::ios_base::failbit);
        }
    }

    [[nodiscard]]
    HANDLE native_handle () const {
        return this->buf.native_handle ();
    }

    [[nodiscard]]
    std::error_code last_error () const {
        return this->buf.last_error ();
    }

  protected:
    void open_file (wchar_t const * p, openmode m,
                    win32_open_options const & o) {
        if (!this->buf.open (p, m, o)) {
            this->setstate (std::ios_base::failbit);
        } else {
            this->clear ();
        }
    }

    void open_file (char const * p, openmode m,
                    win32_open_options const & o) {
        if (!this->buf.open (p, m, o)) {
            this->setstate (std::ios_base::failbit);
        } else {
            this->clear ();
        }
    }

    void open_file (std::wstring const & p, openmode m,
                    win32_open_options const & o) {
        open_file (p.c_str (), m, o);
    }

    void open_file (std::string const & p, openmode m,
                    win32_open_options const & o) {
        open_file (p.c_str (), m, o);
    }
#if defined(LOG4CPLUS_HELPERS_WIN32_FSTREAM_HAS_FILESYSTEM)
    void open_file (std::filesystem::path const & p, openmode m,
                    win32_open_options const & o) {
        if (!this->buf.open (p, m, o)) {
            this->setstate (std::ios_base::failbit);
        } else {
            this->clear ();
        }
    }
#endif
};
} // namespace detail

template <typename CharT, typename Traits = std::char_traits<CharT>,
          typename Codec = utf8_codec<CharT>>
/**
 * @brief Owning bidirectional stream built on `basic_win32_filebuf`.
 *
 * This class follows the familiar `std::basic_fstream` interface while adding
 * configurable Win32 sharing, `native_handle()`, and `last_error()`.
 *
 * ```cpp
 * log4cplus::helpers::win32_wfstream stream(
 *     L"events.log", std::ios::out | std::ios::app);
 * stream << L"ready\n";
 * ```
 *
 * @tparam CharT Character type exposed by the stream.
 * @tparam Traits Character traits for `CharT`.
 * @tparam Codec Converter used by the owned stream buffer.
 */
class basic_win32_fstream
    : public detail::basic_win32_stream_base<
          CharT, Traits, Codec, std::basic_iostream<CharT, Traits>> {
    using base = detail::basic_win32_stream_base<
        CharT, Traits, Codec, std::basic_iostream<CharT, Traits>>;

  public:
    using filebuf_type = typename base::filebuf_type;
    using openmode = typename base::openmode;

    basic_win32_fstream () : base () {
    }

    explicit basic_win32_fstream (
        wchar_t const * p, openmode m = std::ios_base::in | std::ios_base::out,
        win32_open_options const & o = win32_open_options ())
        : base () {
        open (p, m, o);
    }

    explicit basic_win32_fstream (
        char const * p, openmode m = std::ios_base::in | std::ios_base::out,
        win32_open_options const & o = win32_open_options ())
        : base () {
        open (p, m, o);
    }

    explicit basic_win32_fstream (
        std::wstring const & p,
        openmode m = std::ios_base::in | std::ios_base::out,
        win32_open_options const & o = win32_open_options ())
        : base () {
        open (p, m, o);
    }

    explicit basic_win32_fstream (
        std::string const & p,
        openmode m = std::ios_base::in | std::ios_base::out,
        win32_open_options const & o = win32_open_options ())
        : base () {
        open (p, m, o);
    }
#if defined(LOG4CPLUS_HELPERS_WIN32_FSTREAM_HAS_FILESYSTEM)
    explicit basic_win32_fstream (
        std::filesystem::path const & p,
        openmode m = std::ios_base::in | std::ios_base::out,
        win32_open_options const & o = win32_open_options ())
        : base () {
        open (p, m, o);
    }
#endif
    void open (wchar_t const * p,
               openmode m = std::ios_base::in | std::ios_base::out,
               win32_open_options const & o = win32_open_options ()) {
        this->open_file (p, m, o);
    }

    void open (char const * p,
               openmode m = std::ios_base::in | std::ios_base::out,
               win32_open_options const & o = win32_open_options ()) {
        this->open_file (p, m, o);
    }

    void open (std::wstring const & p,
               openmode m = std::ios_base::in | std::ios_base::out,
               win32_open_options const & o = win32_open_options ()) {
        this->open_file (p, m, o);
    }

    void open (std::string const & p,
               openmode m = std::ios_base::in | std::ios_base::out,
               win32_open_options const & o = win32_open_options ()) {
        this->open_file (p, m, o);
    }
#if defined(LOG4CPLUS_HELPERS_WIN32_FSTREAM_HAS_FILESYSTEM)
    void open (std::filesystem::path const & p,
               openmode m = std::ios_base::in | std::ios_base::out,
               win32_open_options const & o = win32_open_options ()) {
        this->open_file (p, m, o);
    }
#endif
};

template <typename CharT, typename Traits = std::char_traits<CharT>,
          typename Codec = utf8_codec<CharT>>
/**
 * @brief Owning output stream built on `basic_win32_filebuf`.
 *
 * This class follows the familiar `std::basic_ofstream` interface while adding
 * configurable Win32 sharing, `native_handle()`, and `last_error()`.
 *
 * @tparam CharT Character type exposed by the stream.
 * @tparam Traits Character traits for `CharT`.
 * @tparam Codec Converter used by the owned stream buffer.
 */
class basic_win32_ofstream
    : public detail::basic_win32_stream_base<
          CharT, Traits, Codec, std::basic_ostream<CharT, Traits>> {
    using base = detail::basic_win32_stream_base<
        CharT, Traits, Codec, std::basic_ostream<CharT, Traits>>;

  public:
    using filebuf_type = typename base::filebuf_type;
    using openmode = typename base::openmode;

    basic_win32_ofstream () : base () {
    }

    explicit basic_win32_ofstream (
        wchar_t const * p, openmode m = std::ios_base::out,
        win32_open_options const & o = win32_open_options ())
        : base () {
        open (p, m, o);
    }

    explicit basic_win32_ofstream (
        char const * p, openmode m = std::ios_base::out,
        win32_open_options const & o = win32_open_options ())
        : base () {
        open (p, m, o);
    }

    explicit basic_win32_ofstream (
        std::wstring const & p, openmode m = std::ios_base::out,
        win32_open_options const & o = win32_open_options ())
        : base () {
        open (p, m, o);
    }

    explicit basic_win32_ofstream (
        std::string const & p, openmode m = std::ios_base::out,
        win32_open_options const & o = win32_open_options ())
        : base () {
        open (p, m, o);
    }
#if defined(LOG4CPLUS_HELPERS_WIN32_FSTREAM_HAS_FILESYSTEM)
    explicit basic_win32_ofstream (
        std::filesystem::path const & p, openmode m = std::ios_base::out,
        win32_open_options const & o = win32_open_options ())
        : base () {
        open (p, m, o);
    }
#endif
    void open (wchar_t const * p, openmode m = std::ios_base::out,
               win32_open_options const & o = win32_open_options ()) {
        this->open_file (p, m | std::ios_base::out, o);
    }

    void open (char const * p, openmode m = std::ios_base::out,
               win32_open_options const & o = win32_open_options ()) {
        this->open_file (p, m | std::ios_base::out, o);
    }

    void open (std::wstring const & p, openmode m = std::ios_base::out,
               win32_open_options const & o = win32_open_options ()) {
        this->open_file (p, m | std::ios_base::out, o);
    }

    void open (std::string const & p, openmode m = std::ios_base::out,
               win32_open_options const & o = win32_open_options ()) {
        this->open_file (p, m | std::ios_base::out, o);
    }
#if defined(LOG4CPLUS_HELPERS_WIN32_FSTREAM_HAS_FILESYSTEM)
    void open (std::filesystem::path const & p,
               openmode m = std::ios_base::out,
               win32_open_options const & o = win32_open_options ()) {
        this->open_file (p, m | std::ios_base::out, o);
    }
#endif
};

/** @brief Locale-encoded narrow-character Win32 stream. */
using win32_fstream = basic_win32_fstream<char>;
/** @brief Locale-encoded narrow-character Win32 output stream. */
using win32_ofstream = basic_win32_ofstream<char>;
/** @brief Windows wide-character Win32 stream. */
using win32_wfstream = basic_win32_fstream<wchar_t>;
/** @brief Windows wide-character Win32 output stream. */
using win32_wofstream = basic_win32_ofstream<wchar_t>;
/** @brief UTF-16 Win32 stream. */
using win32_u16fstream = basic_win32_fstream<char16_t>;
/** @brief UTF-16 Win32 output stream. */
using win32_u16ofstream = basic_win32_ofstream<char16_t>;
/** @brief UTF-32 Win32 stream. */
using win32_u32fstream = basic_win32_fstream<char32_t>;
/** @brief UTF-32 Win32 output stream. */
using win32_u32ofstream = basic_win32_ofstream<char32_t>;
#if defined(__cpp_char8_t)
/** @brief C++20 UTF-8 character Win32 stream. */
using win32_u8fstream = basic_win32_fstream<char8_t>;
/** @brief C++20 UTF-8 character Win32 output stream. */
using win32_u8ofstream = basic_win32_ofstream<char8_t>;
#endif

} } // namespace log4cplus::helpers
#endif // _WIN32
#endif // LOG4CPLUS_HELPERS_WIN32_FSTREAM_HEADER_
