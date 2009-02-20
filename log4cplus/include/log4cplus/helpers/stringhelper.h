// Module:  Log4CPLUS
// File:    stringhelper.h
// Created: 3/2003
// Author:  Tad E. Smith
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

/** @file */

#ifndef LOG4CPLUS_HELPERS_STRINGHELPER_HEADER_
#define LOG4CPLUS_HELPERS_STRINGHELPER_HEADER_

#include <log4cplus/config.hxx>
#include <log4cplus/tstring.h>

#include <algorithm>
#include <limits>
#include <iterator>


namespace log4cplus {
    namespace helpers {

        /**
         * Returns <code>s</code> in upper case.
         */
        LOG4CPLUS_EXPORT log4cplus::tstring toUpper(const log4cplus::tstring& s);


        /**
         * Returns <code>s</code> in lower case.
         */
        LOG4CPLUS_EXPORT log4cplus::tstring toLower(const log4cplus::tstring& s);


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
        template <class StringType, class OutputIter>
        void tokenize(const StringType& s, typename StringType::value_type c, 
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
        }
        

        template<class intType>
        inline
        tstring 
        convertIntegerToString (intType value) 
        {
            if (value == 0)
                return LOG4CPLUS_TEXT("0");
            bool const negative = value < 0;

            static const size_t buffer_size
                = std::numeric_limits<intType>::digits10 + 2;
            tchar buffer[buffer_size];           
            tchar * it = &buffer[buffer_size];
            tchar const * const buf_end = it;

            // The sign of the result of the modulo operator is implementation
            // defined. That's why we work with positive counterpart instead.
            // Also, in twos complement arithmetic the smallest negative number
            // does not have positive counterpart; the range is asymetric.
            // That's why we handle the case of value == min() specially here.
            if (negative)
            {
                // The modulo operator on 
                if (value == (std::numeric_limits<intType>::min) ())
                {
                    intType const r = value / 10;
                    intType const a = (-r) * 10;
                    intType const mod = -(a + value);
                    value = -r;
                    *(it - 1) = LOG4CPLUS_TEXT('0') + static_cast<tchar>(mod);
                    --it;
                }
                else
                    value = -value;
            }

            for (; value != 0; --it)
            {
                intType mod = value % 10;
                value = value / 10;
                *(it - 1) = LOG4CPLUS_TEXT('0') + static_cast<tchar>(mod);
            }

            if (negative)
            {
                --it;
                *it = LOG4CPLUS_TEXT('-');
            }

            return tstring (static_cast<tchar const *>(it), buf_end);
        }


        /**
         * This iterator can be used in place of the back_insert_iterator
         * for compilers that don't have a std::basic_string class that
         * has the <code>push_back</code> method.
         */
        template <class Container>
        class string_append_iterator
            : public std::iterator<std::output_iterator_tag, void, void, void,
                void>
        {
        public:
            typedef Container container_type;

            explicit string_append_iterator(container_type & c)
                : container(&c)
            { }

            string_append_iterator<container_type> &
            operator = (const typename container_type::value_type& value)
            {
                *container += value;
                return *this;
            }

            string_append_iterator<container_type> &
            operator * ()
            {
                return *this;
            }
            
            string_append_iterator<container_type> &
            operator ++ ()
            {
                return *this;
            }

            string_append_iterator<container_type>
            operator ++ (int)
            {
                return *this;
            }

        protected:
            container_type * container;
        };

    } // namespace helpers

} //namespace log4cplus


#endif // LOG4CPLUS_HELPERS_STRINGHELPER_HEADER_
