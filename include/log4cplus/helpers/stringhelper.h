// Module:  Log4CPLUS
// File:    stringhelper.h
// Created: 3/2003
// Author:  Tad E. Smith
//
//
// Copyright (C) The Apache Software Foundation. All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

/** @file */

#ifndef LOG4CPLUS_HELPERS_STRINGHELPER_HEADER_
#define LOG4CPLUS_HELPERS_STRINGHELPER_HEADER_

#include <log4cplus/config.h>

#include <string>
#include <iterator>
#include <cctype>

namespace log4cplus {

    namespace helpers {

        /**
         * Returns <code>s</code> in upper case.
         */
        template <class _StringType>
        _StringType toupper(const _StringType& s)
        {
            _StringType ret;
            transform(s.begin(), s.end(),
                      std::back_insert_iterator<_StringType>(ret),
                      ::toupper);
            return ret;
        }


        /**
         * Returns <code>s</code> in lower case.
         */
        template <class _StringType>
        _StringType tolower(const _StringType& s)
        {
            _StringType ret;
            transform(s.begin(), s.end(),
                      std::back_insert_iterator<_StringType>(ret),
                      ::tolower);
            return ret;
        }


        /**
         * Tokenize <code>s</code> using <code>c</code> as the delimiter and
         * put the resulting tokens in <code>_result</code>.  If 
         * <code>collapseTokens</code> is false, multiple adjacent delimiters
         * will result in zero length tokens.
         * <p>
         * <b>Example:</b>
         * <pre>
         *   string s = // Set string with '.' as delimiters
         *   list<std::string> tokens;
         *   tokenize(s, '.', back_insert_iterator<list<string> >(tokens));
         * </pre>
         */
        template <class _StringType, class _OutputIter>
        void tokenize(const _StringType& s, typename _StringType::value_type c, 
                      _OutputIter _result, bool collapseTokens = true) 
        {
            _StringType tmp;
            for(int i=0, count=0; i<s.length(); ++i) {
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

    } 
}


#endif // LOG4CPLUS_HELPERS_STRINGHELPER_HEADER_

