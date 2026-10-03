// -*- C++ -*-
// Module:  Log4CPLUS
// File:    androidappender.h
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

/** @file */

#ifndef LOG4CPLUS_ANDROID_APPENDER_HEADER_
#define LOG4CPLUS_ANDROID_APPENDER_HEADER_

#include <log4cplus/config.hxx>

#if defined (LOG4CPLUS_HAVE_PRAGMA_ONCE)
#pragma once
#endif

#if defined (__ANDROID__)

#include <log4cplus/appender.h>
#include <string>


namespace log4cplus
{

    /**
     * Sends formatted log events to Android's main Logcat buffer.
     *
     * <h3>Properties</h3>
     * <dl>
     * <dt><tt>Tag</tt></dt>
     * <dd>Logcat tag attached to each event. The default is
     * <tt>log4cplus</tt>. Tags are limited to 23 UTF-8 bytes for
     * compatibility with Android API levels 21 through 25.</dd>
     * </dl>
     */
    class LOG4CPLUS_EXPORT AndroidAppender
        : public Appender
    {
    public:
        explicit AndroidAppender (
            tstring const & tag = LOG4CPLUS_TEXT ("log4cplus"));
        explicit AndroidAppender (helpers::Properties const & properties);
        ~AndroidAppender () override;

        void close () override;

        AndroidAppender (AndroidAppender const &) = delete;
        AndroidAppender & operator = (AndroidAppender const &) = delete;

    protected:
        void append (spi::InternalLoggingEvent const & event) override;

    private:
        std::string tag;
    };

} // namespace log4cplus

#endif // defined (__ANDROID__)
#endif // LOG4CPLUS_ANDROID_APPENDER_HEADER_
