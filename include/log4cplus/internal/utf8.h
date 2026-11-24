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

#ifndef LOG4CPLUS_INTERNAL_UTF8_HEADER_
#define LOG4CPLUS_INTERNAL_UTF8_HEADER_

#include <cstddef>
#include <cstdint>

namespace log4cplus { namespace helpers { namespace detail {

constexpr std::uint32_t unicode_replacement_character = 0xfffd;
constexpr std::size_t maximum_utf8_sequence_length = 4;
constexpr std::uint32_t high_surrogate_first = 0xd800;
constexpr std::uint32_t high_surrogate_last = 0xdbff;
constexpr std::uint32_t low_surrogate_first = 0xdc00;
constexpr std::uint32_t low_surrogate_last = 0xdfff;

/** @brief Tests whether a code unit is a UTF-16 high surrogate. */
constexpr bool is_high_surrogate (std::uint32_t c) noexcept {
    return high_surrogate_first <= c && c <= high_surrogate_last;
}

/** @brief Tests whether a code unit is a UTF-16 low surrogate. */
constexpr bool is_low_surrogate (std::uint32_t c) noexcept {
    return low_surrogate_first <= c && c <= low_surrogate_last;
}

/** @brief Tests whether a value is a Unicode scalar value. */
constexpr bool is_scalar (std::uint32_t c) noexcept {
    return c <= 0x10ffff && !is_high_surrogate (c) && !is_low_surrogate (c);
}

/** Encodes a valid Unicode scalar into a buffer of at least
 * maximum_utf8_sequence_length bytes. Returns the number of bytes written.
 */
constexpr std::size_t encode_utf8 (char * out, std::uint32_t c) noexcept {
    // Distribute the scalar's payload bits across UTF-8 code units.
    if (c <= 0x7f) {
        out[0] = static_cast<char> (c);
        return 1;
    } else if (c <= 0x7ff) {
        out[0] = static_cast<char> (0xc0 | (c >> 6));
        out[1] = static_cast<char> (0x80 | (c & 0x3f));
        return 2;
    } else if (c <= 0xffff) {
        out[0] = static_cast<char> (0xe0 | (c >> 12));
        out[1] = static_cast<char> (0x80 | ((c >> 6) & 0x3f));
        out[2] = static_cast<char> (0x80 | (c & 0x3f));
        return 3;
    } else {
        out[0] = static_cast<char> (0xf0 | (c >> 18));
        out[1] = static_cast<char> (0x80 | ((c >> 12) & 0x3f));
        out[2] = static_cast<char> (0x80 | ((c >> 6) & 0x3f));
        out[3] = static_cast<char> (0x80 | (c & 0x3f));
        return 4;
    }
}

} } } // namespace log4cplus::helpers::detail

#endif // LOG4CPLUS_INTERNAL_UTF8_HEADER_
