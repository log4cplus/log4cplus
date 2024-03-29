// Module:  Log4CPLUS
// File:    consoleappender.cxx
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

#include <log4cplus/layout.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/streams.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/internal/env.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/thread/syncprims-pub-impl.h>
#include <ostream>


namespace log4cplus
{


namespace helpers
{

extern log4cplus::thread::Mutex const & getConsoleOutputMutex ();

} // namespace helpers


log4cplus::thread::Mutex const &
ConsoleAppender::getOutputMutex ()
{
    return helpers::getConsoleOutputMutex ();
}


//////////////////////////////////////////////////////////////////////////////
// ConsoleAppender ctors and dtor
//////////////////////////////////////////////////////////////////////////////

ConsoleAppender::ConsoleAppender(bool logToStdErr_,
    bool immediateFlush_)
: logToStdErr(logToStdErr_),
  immediateFlush(immediateFlush_),
  locale(nullptr)
{
}



ConsoleAppender::ConsoleAppender(const helpers::Properties & properties)
: Appender(properties),
  logToStdErr(false),
  immediateFlush(false),
  locale(nullptr)
{
    properties.getBool (logToStdErr, LOG4CPLUS_TEXT("logToStdErr"));
    properties.getBool (immediateFlush, LOG4CPLUS_TEXT("ImmediateFlush"));

    tstring localeName;
    if (properties.getString(localeName, LOG4CPLUS_TEXT("Locale"))) {
        locale = std::make_unique<std::locale>(internal::get_locale_by_name(localeName));
        // we need to flash immediately if non-default locale is used
        immediateFlush = true;
    }
}



ConsoleAppender::~ConsoleAppender()
{
    destructorImpl();
}



//////////////////////////////////////////////////////////////////////////////
// ConsoleAppender public methods
//////////////////////////////////////////////////////////////////////////////

void 
ConsoleAppender::close()
{
    helpers::getLogLog().debug(
        LOG4CPLUS_TEXT("Entering ConsoleAppender::close().."));
    closed = true;
}



//////////////////////////////////////////////////////////////////////////////
// ConsoleAppender protected methods
//////////////////////////////////////////////////////////////////////////////

void
ConsoleAppender::append(const spi::InternalLoggingEvent& event)
{
    thread::MutexGuard guard (getOutputMutex ());

    tostream& output = (logToStdErr ? tcerr : tcout);

    std::locale cur_loc;
    if (locale != nullptr) {
        cur_loc = output.getloc();
        output.imbue(*locale);
    }
    layout->formatAndAppend(output, event);
    if(immediateFlush) {
        output.flush();
    }
    if (locale != nullptr) {
        output.imbue(cur_loc);
    }
}


} // namespace log4cplus
