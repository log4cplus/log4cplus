// Module:  Log4CPLUS
// File:    loggingmacros.cxx
// Created: 4/2010
// Author:  Vaclav Haisman
//
//
// Copyright 2010 Vaclav Haisman
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

/** @file 
 * This file implements support function for loggingmacros.h file. */

#include <log4cplus/internal/internal.h>


namespace log4cplus { namespace detail {


macro_body_scratch_pad_type::macro_body_scratch_pad_type (tostringstream & s,
    spi::InternalLoggingEvent & e)
    : oss (s)
    , ev (e)
{ }


log4cplus::spi::InternalLoggingEvent &
get_macros_event ()
{
    return internal::get_ptd ()->forced_log_ev;
}


macro_body_scratch_pad_type &
get_macro_body_scratch_pad ()
{
    macro_body_scratch_pad_type & sp
        = internal::get_ptd ()->macro_body_scratch_pad;
    clear_tostringstream (sp.oss);
    return sp;
}


//! Helper stream to get the defaults from.
static tostringstream const macros_oss_defaults;

// Individual defaults.

static std::ios_base::fmtflags const default_flags
    = macros_oss_defaults.flags ();
static log4cplus::tchar const default_fill = macros_oss_defaults.fill ();
static std::streamsize const default_precision
    = macros_oss_defaults.precision ();
static std::streamsize const default_width = macros_oss_defaults.width ();

//! Clears string stream using defaults taken from macros_oss_defaults.
void
clear_tostringstream (tostringstream & os)
{
    os.clear ();
    os.str (internal::empty_str);
    os.setf (default_flags);
    os.fill (default_fill);
    os.precision (default_precision);
    os.width (default_width);
#if defined (LOG4CPLUS_WORKING_LOCALE)
    std::locale glocale = std::locale ();
    if (os.getloc () != glocale)
        os.imbue (glocale);
#endif // defined (LOG4CPLUS_WORKING_LOCALE)
}


void
macro_forced_log (log4cplus::Logger const & logger,
    log4cplus::LogLevel log_level, log4cplus::tstring const & msg,
    char const * filename, int line, char const * func)
{
    log4cplus::spi::InternalLoggingEvent & ev = internal::get_ptd ()->forced_log_ev;
    ev.setLoggingEvent (logger.getName (), log_level, msg, filename, line);
    ev.setFunction (func);
    logger.forcedLog (ev);
}


} } // namespace log4cplus { namespace detail {
