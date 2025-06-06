// -*- C++ -*-
// Module:  Log4CPLUS
// File:    stringhelper.h
// Created: 3/2003
// Author:  Tad E. Smith
//
//
// Copyright 2003-2017 Tad E. Smith
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

/** @file */

#ifndef LOG4CPLUS_HELPERS_STRINGHELPER_HEADER_
#define LOG4CPLUS_HELPERS_STRINGHELPER_HEADER_

#include <log4cplus/config.hxx>

#if defined (LOG4CPLUS_HAVE_PRAGMA_ONCE)
#pragma once
#endif

#include <log4cplus/tstring.h>

#include <algorithm>
#include <limits>


namespace log4cplus {
    namespace helpers {

        /**
         * Returns <code>s</code> in upper case.
         */
        LOG4CPLUS_EXPORT log4cplus::tstring toUpper(const log4cplus::tstring_view& s);
        LOG4CPLUS_EXPORT tchar toUpper(tchar);


        /**
         * Returns <code>s</code> in lower case.
         */
        LOG4CPLUS_EXPORT log4cplus::tstring toLower(const log4cplus::tstring_view& s);
        LOG4CPLUS_EXPORT tchar toLower(tchar);


        /**
         * Tokenize <code>s</code> using <code>c</code> as the delimiter and
         * put the resulting tokens in <code>_result</code>.  If
         * <code>collapseTokens</code> is false, multiple adjacent delimiters
         * will result in zero length tokens.
         *
         * <b>Example:</b>
         * <pre>
         *   string s = // Set string with '.' as delimiters
         *   list<log4cplus::tstring> tokens;
         *   tokenize(s, '.', back_insert_iterator<list<string> >(tokens));
         * </pre>
         */
        template <typename StringType, typename OutputIter>
            requires std::output_iterator<OutputIter, StringType>
        inline
        void
        tokenize(const StringType& s, typename StringType::value_type c,
            OutputIter result, bool collapseTokens = true)
        {
            typedef typename StringType::size_type size_type;
            size_type const slen = s.length();
            size_type first = 0;
            size_type i = 0;
            for (i=0; i < slen; ++i)
            {
                if (s[i] == c)
                {
                    *result = StringType (s, first, i - first);
                    ++result;
                    if (collapseTokens)
                        while (i+1 < slen && s[i+1] == c)
                            ++i;
                    first = i + 1;
                }
            }
            if (first != i)
                *result = StringType (s, first, i - first);
            else if (! collapseTokens && first == i)
                *result = StringType ();
        }


        template <std::integral intType, typename stringType, bool isSigned>
        struct ConvertIntegerToStringHelper;


        template <std::integral intType, typename charType>
        struct ConvertIntegerToStringHelper<intType, charType, true>
        {
            static inline
            void
            step1 (charType * & it, intType & value)
            {
                // The sign of the result of the modulo operator is
                // implementation defined. That's why we work with
                // positive counterpart instead.  Also, in twos
                // complement arithmetic the smallest negative number
                // does not have positive counterpart; the range is
                // asymmetric.  That's why we handle the case of value
                // == min() specially here.
                if (value == (std::numeric_limits<intType>::min) ()) [[unlikely]]
                {
                    intType const r = value / 10;
                    intType const a = (-r) * 10;
                    intType const mod = -(a + value);
                    value = -r;

                    *(it - 1)
                        = static_cast<charType>(LOG4CPLUS_TEXT('0') + mod);
                    --it;
                }
                else
                    value = -value;
            }

            static constexpr
            bool
            is_negative (intType val)
            {
                return val < 0;
            }
        };


        template <std::integral intType, typename charType>
        struct ConvertIntegerToStringHelper<intType, charType, false>
        {
            static inline
            void
            step1 (charType * &, intType &)
            {
                // This will never be called for unsigned types.
            }

            static constexpr
            bool
            is_negative (intType)
            {
                return false;
            }
        };


        template <class stringType, std::integral intType>
        inline
        void
        convertIntegerToString (stringType & str, intType value)
        {
            typedef std::numeric_limits<intType> intTypeLimits;
            typedef typename stringType::value_type charType;
            typedef ConvertIntegerToStringHelper<intType, charType,
                intTypeLimits::is_signed> HelperType;

            charType buffer[intTypeLimits::digits10 + 2];
            const std::size_t buffer_size
                = sizeof (buffer) / sizeof (charType);

            charType * it = &buffer[buffer_size];
            charType const * const buf_end = &buffer[buffer_size];

            if (value == 0) [[unlikely]]
            {
                --it;
                *it = LOG4CPLUS_TEXT('0');
            }
            else
            {
                bool const negative = HelperType::is_negative (value);
                if (negative)
                    HelperType::step1 (it, value);

                for (; value != 0; --it)
                {
                    intType mod = value % 10;
                    value = value / 10;
                    *(it - 1) = static_cast<charType>(LOG4CPLUS_TEXT('0')
                        + mod);
                }

                if (negative)
                {
                    --it;
                    *it = LOG4CPLUS_TEXT('-');
                }
            }

            str.assign (static_cast<charType const *>(it), buf_end);
        }


        template<std::integral intType>
        inline
        tstring
        convertIntegerToString (intType value)
        {
            tstring result;
            convertIntegerToString (result, value);
            return result;
        }


        template<std::integral intType>
        inline
        std::string
        convertIntegerToNarrowString (intType value)
        {
            std::string result;
            convertIntegerToString (result, value);
            return result;
        }


        //! Join a list of items into a string.
        template <typename Iterator, typename Separator>
            requires std::forward_iterator<Iterator>
        inline
        void
        join_worker (tstring & result, Iterator & start, Iterator & last,
            Separator const & sep)
        {
            if (start != last)
                result = *start++;

            for (; start != last; ++start)
            {
                result += sep;
                result += *start;
            }
        }

        //! Join a list of items into a string.
        template <typename Iterator>
            requires std::forward_iterator<Iterator>
        inline
        void
        join (tstring & result, Iterator start, Iterator last,
            tstring_view const & sep)
        {
            join_worker (result, start, last, sep);
        }

        //! Join a list of items into a string.
        template <typename Iterator>
            requires std::forward_iterator<Iterator>
        inline
        void
        join (tstring & result, Iterator start, Iterator last,
            tstring::value_type sep)
        {
            join_worker (result, start, last, sep);
        }


    } // namespace helpers

} // namespace log4cplus

#endif // LOG4CPLUS_HELPERS_STRINGHELPER_HEADER_
