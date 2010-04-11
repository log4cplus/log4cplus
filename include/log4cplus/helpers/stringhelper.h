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

#include <log4cplus/config.h>
#include <log4cplus/tstring.h>

#include <algorithm>


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
         * <p>
         * <b>Example:</b>
         * <pre>
         *   string s = // Set string with '.' as delimiters
         *   list<log4cplus::tstring> tokens;
         *   tokenize(s, '.', back_insert_iterator<list<string> >(tokens));
         * </pre>
         */
        template <class _StringType, class _OutputIter>
        void tokenize(const _StringType& s, typename _StringType::value_type c, 
                      _OutputIter _result, bool collapseTokens = true) 
        {
            _StringType tmp;
            for(typename _StringType::size_type i=0; i<s.length(); ++i) {
                if(s[i] == c) {
                    *_result = tmp;
                    ++_result;
                    tmp.erase(tmp.begin(), tmp.end());
                    if(collapseTokens)
                        while(s[i+1] == c) ++i;
                }
                else
                    tmp += s[i];
            }
            if(tmp.length() > 0) *_result = tmp;
        }
        
        
         
        template<class intType>
        inline tstring convertIntegerToString(intType value) 
        {
            if(value == 0) {
                return LOG4CPLUS_TEXT("0");
            }
            
            char buffer[21];
            char ret[21];
            unsigned int bufferPos = 0;
            unsigned int retPos = 0;

            if(value < 0) {
                ret[retPos++] = '-';
            }
            
            // convert to string in reverse order
            while(value != 0) {
                intType mod = value % 10;
                value = value / 10;
                buffer[bufferPos++] = '0' + static_cast<char>(mod);
            }
            
            // now reverse the string to get it in proper order
            while(bufferPos > 0) {
                ret[retPos++] = buffer[--bufferPos];
            }
            ret[retPos] = 0;
            
            return LOG4CPLUS_C_STR_TO_TSTRING(ret);
        }


        /**
         * This iterator can be used in place of the back_insert_iterator
         * for compilers that don't have a std::basic_string class that
         * has the <code>push_back</code> method.
         */
        template <class _Container>
        class string_append_iterator {
        protected:
            _Container* container;
        public:
            typedef _Container          container_type;
            typedef void                value_type;
            typedef void                difference_type;
            typedef void                pointer;
            typedef void                reference;

            explicit string_append_iterator(_Container& __x) : container(&__x) {}
            string_append_iterator<_Container>&
            operator=(const typename _Container::value_type& __value) {
                *container += __value;
                return *this;
            }
            string_append_iterator<_Container>& operator*() { return *this; }
            string_append_iterator<_Container>& operator++() { return *this; }
            string_append_iterator<_Container>& operator++(int) { return *this; }
        };

    } 
}

#endif // LOG4CPLUS_HELPERS_STRINGHELPER_HEADER_

