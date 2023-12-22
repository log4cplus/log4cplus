// -*- C++ -*-
// Module:  Log4CPLUS
// File:    consoleappender.h
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright 2001-2017 Tad E. Smith
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

#ifndef LOG4CPLUS_CONSOLE_APPENDER_HEADER_
#define LOG4CPLUS_CONSOLE_APPENDER_HEADER_

#include <log4cplus/config.hxx>

#if defined (LOG4CPLUS_HAVE_PRAGMA_ONCE)
#pragma once
#endif

#include <log4cplus/appender.h>
#include <locale>

namespace log4cplus {
    /**
     * ConsoleAppender appends log events to <code>std::cout</code> or
     * <code>std::cerr</code> using a layout specified by the
     * user. The default target is <code>std::cout</code>.
     *
     * <h3>Properties</h3>
     * <dl>
     * <dt><tt>logToStdErr</tt></dt>
     * <dd>When it is set true, the output stream will be
     * <code>std::cerr</code> instead of <code>std::cout</code>.</dd>
     *
     * <dt><tt>ImmediateFlush</tt></dt>
     * <dd>When it is set true, output stream will be flushed after
     * each appended event.</dd>
     * 
     * <dt><tt>Locale</tt></dt>
     * <dd>This property specifies a locale name that will be imbued
     * into output stream. Locale can be specified either by system
     * specific locale name, e.g., <tt>en_US.UTF-8</tt>, or by one of
     * four recognized keywords: <tt>GLOBAL</tt>, <tt>DEFAULT</tt>
     * (which is an alias for <tt>GLOBAL</tt>), <tt>USER</tt> and
     * <tt>CLASSIC</tt>. When specified locale is not available,
     * <tt>GLOBAL</tt> is used instead. It is possible to register
     * additional locale keywords by registering an instance of
     * <code>spi::LocaleFactory</code> in
     * <code>spi::LocaleFactoryRegistry</code>.
     * \sa spi::getLocaleFactoryRegistry().
     * 
     * Note: if <tt>Locale</tt> is set, <tt>ImmediateFlush</tt> will
     * be set to true automatically.
     * </dd>
     * 
     * </dl>
     * \sa Appender
     */
    class LOG4CPLUS_EXPORT ConsoleAppender : public Appender {
    public:
      // Ctors
        ConsoleAppender(bool logToStdErr = false, bool immediateFlush = false);
        ConsoleAppender(const log4cplus::helpers::Properties & properties);

      // Dtor
        ~ConsoleAppender();

      // Methods
        virtual void close();

        //! This mutex is used by ConsoleAppender and helpers::LogLog
        //! classes to synchronize output to console.
        static log4cplus::thread::Mutex const & getOutputMutex();

    protected:
        virtual void append(const spi::InternalLoggingEvent& event);

      // Data
        bool logToStdErr;
        /**
         * Immediate flush means that the underlying output stream
         * will be flushed at the end of each append operation.
         */
        bool immediateFlush;

        std::unique_ptr<std::locale> locale;
    };

} // end namespace log4cplus

#endif // LOG4CPLUS_CONSOLE_APPENDER_HEADER_

