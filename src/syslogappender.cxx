// Module:  Log4CPLUS
// File:    syslogappender.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
// Copyright 2001-2010 Tad E. Smith
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

#include <log4cplus/syslogappender.h>
#if defined(LOG4CPLUS_HAVE_SYSLOG_H) && !defined(_WIN32)

#include <log4cplus/streams.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/internal/internal.h>
#include <log4cplus/internal/env.h>
#include <log4cplus/thread/syncprims-pub-impl.h>
#include <syslog.h>
#include <cstring>


namespace log4cplus
{

namespace
{

static
const char*
useIdent (const std::string& string)
{
    if (string.empty ())
        return 0;
    else
        return string.c_str ();
}


static
int
parseFacility (const tstring& text)
{
    if (text.empty ())
        return 0;
#ifdef LOG_AUTH
    else if (text == LOG4CPLUS_TEXT ("auth"))
        return LOG_AUTH;
#endif
#ifdef LOG_AUTHPRIV
    else if (text == LOG4CPLUS_TEXT ("authpriv"))
        return LOG_AUTHPRIV;
#endif
#ifdef LOG_CRON
    else if (text == LOG4CPLUS_TEXT ("cron"))
        return LOG_CRON;
#endif
#ifdef LOG_DAEMON
    else if (text == LOG4CPLUS_TEXT ("daemon"))
        return LOG_DAEMON;
#endif
#ifdef LOG_FTP
    else if (text == LOG4CPLUS_TEXT ("ftp"))
        return LOG_FTP;
#endif
#ifdef LOG_KERN
    else if (text == LOG4CPLUS_TEXT ("kern"))
        return LOG_KERN;
#endif
#ifdef LOG_LOCAL0
    else if (text == LOG4CPLUS_TEXT ("local0"))
        return LOG_LOCAL0;
#endif
#ifdef LOG_LOCAL1
    else if (text == LOG4CPLUS_TEXT ("local1"))
        return LOG_LOCAL1;
#endif
#ifdef LOG_LOCAL2
    else if (text == LOG4CPLUS_TEXT ("local2"))
        return LOG_LOCAL2;
#endif
#ifdef LOG_LOCAL3
    else if (text == LOG4CPLUS_TEXT ("local3"))
        return LOG_LOCAL3;
#endif
#ifdef LOG_LOCAL4
    else if (text == LOG4CPLUS_TEXT ("local4"))
        return LOG_LOCAL4;
#endif
#ifdef LOG_LOCAL5
    else if (text == LOG4CPLUS_TEXT ("local5"))
        return LOG_LOCAL5;
#endif
#ifdef LOG_LOCAL6
    else if (text == LOG4CPLUS_TEXT ("local6"))
        return LOG_LOCAL6;
#endif
#ifdef LOG_LOCAL7
    else if (text == LOG4CPLUS_TEXT ("local7"))
        return LOG_LOCAL7;
#endif
#ifdef LOG_LPR
    else if (text == LOG4CPLUS_TEXT ("lpr"))
        return LOG_LPR;
#endif
#ifdef LOG_MAIL
    else if (text == LOG4CPLUS_TEXT ("mail"))
        return LOG_MAIL;
#endif
#ifdef LOG_NEWS
    else if (text == LOG4CPLUS_TEXT ("news"))
        return LOG_NEWS;
#endif
#ifdef LOG_SYSLOG
    else if (text == LOG4CPLUS_TEXT ("syslog"))
        return LOG_SYSLOG;
#endif
#ifdef LOG_USER
    else if (text == LOG4CPLUS_TEXT ("user"))
        return LOG_USER;
#endif
#ifdef LOG_UUCP
    else if (text == LOG4CPLUS_TEXT ("uucp"))
        return LOG_UUCP;
#endif
    else
    {
        // Unknown facility.
        tstring msg (LOG4CPLUS_TEXT ("Unknown syslog facility: "));
        msg += text;
        helpers::getLogLog ().error (msg);

        return 0;
    }
}


} // namespace


///////////////////////////////////////////////////////////////////////////////
// SysLogAppender ctors and dtor
///////////////////////////////////////////////////////////////////////////////

SysLogAppender::SysLogAppender(const tstring& id)
    : ident(id)
    , facility (0)
    , appendFunc (&SysLogAppender::appendLocal)
    , port (0)
    // Store std::string form of ident as member of SysLogAppender so
    // the address of the c_str() result remains stable for openlog &
    // co to use even if we use wstrings.
    , identStr(LOG4CPLUS_TSTRING_TO_STRING (id) )
{
    ::openlog(useIdent(identStr), 0, 0);
}


SysLogAppender::SysLogAppender(const helpers::Properties & properties)
    : Appender(properties)
    , facility (0)
    , appendFunc (0)
    , port (0)
{
    ident = properties.getProperty( LOG4CPLUS_TEXT("ident") );
    facility = parseFacility (
        helpers::toLower (
            properties.getProperty (LOG4CPLUS_TEXT ("facility"))));
    identStr = LOG4CPLUS_TSTRING_TO_STRING (ident);

    host = properties.getProperty (LOG4CPLUS_TEXT ("host"));
    if (host.empty ())
    {        
        appendFunc = &SysLogAppender::appendLocal;
        ::openlog(useIdent(identStr), 0, 0);
    }
    else
    {
        if (! properties.getInt (port, LOG4CPLUS_TEXT ("port")))
            port = 514;
        
        appendFunc = &SysLogAppender::appendRemote;
        syslogSocket = helpers::Socket (host, port, true);        
    }
}


SysLogAppender::SysLogAppender(const tstring& id, const tstring & h,
    int p)
    : ident (id)
    , facility (0)
    , appendFunc (&SysLogAppender::appendRemote)
    , host (h)
    , port (p)
    , syslogSocket (host, port, true)
    // Store std::string form of ident as member of SysLogAppender so
    // the address of the c_str() result remains stable for openlog &
    // co to use even if we use wstrings.
    , identStr(LOG4CPLUS_TSTRING_TO_STRING (id) )
{ }


SysLogAppender::~SysLogAppender()
{
    destructorImpl();
}



///////////////////////////////////////////////////////////////////////////////
// SysLogAppender public methods
///////////////////////////////////////////////////////////////////////////////

void 
SysLogAppender::close()
{
    helpers::getLogLog().debug(
        LOG4CPLUS_TEXT("Entering SysLogAppender::close()..."));
    thread::MutexGuard guard (access_mutex);

    ::closelog();
    closed = true;
}



///////////////////////////////////////////////////////////////////////////////
// SysLogAppender protected methods
///////////////////////////////////////////////////////////////////////////////

int
SysLogAppender::getSysLogLevel(const LogLevel& ll) const
{
    if(ll < INFO_LOG_LEVEL /* || ll < DEBUG_LOG_LEVEL*/) {
        return LOG_DEBUG;
    }
    else if(ll < WARN_LOG_LEVEL) {
        return LOG_INFO;
    }
    else if(ll < ERROR_LOG_LEVEL) {
        return LOG_WARNING;
    }
    else if(ll < FATAL_LOG_LEVEL) {
        return LOG_ERR;
    }
    else if(ll == FATAL_LOG_LEVEL) {
        return LOG_CRIT;
    }

    return LOG_ALERT;  // ll > FATAL_LOG_LEVEL
}


// This method does not need to be locked since it is called by
// doAppend() which performs the locking
void
SysLogAppender::append(const spi::InternalLoggingEvent& event)
{
    (this->*appendFunc) (event);
}


void
SysLogAppender::appendLocal(const spi::InternalLoggingEvent& event)
{
    int const level = getSysLogLevel(event.getLogLevel());
    internal::appender_sratch_pad & appender_sp = internal::get_appender_sp ();
    detail::clear_tostringstream (appender_sp.oss);
    layout->formatAndAppend(appender_sp.oss, event);
    appender_sp.str = appender_sp.oss.str ();
    ::syslog(facility | level, "%s",
        LOG4CPLUS_TSTRING_TO_STRING(appender_sp.str).c_str());
}


tstring const SysLogAppender::remoteTimeFormat (
    LOG4CPLUS_TEXT ("%Y-%m-%dT%H:%M:%S.%qZ"));


void
SysLogAppender::appendRemote(const spi::InternalLoggingEvent& event)
{
    int const level = getSysLogLevel(event.getLogLevel());
    internal::appender_sratch_pad & appender_sp = internal::get_appender_sp ();
    detail::clear_tostringstream (appender_sp.oss);

    appender_sp.oss
        // PRI
        << LOG4CPLUS_TEXT ('<') << (level | facility) << LOG4CPLUS_TEXT ('>')
        // VERSION
        << 1
        // TIMESTAMP
        << LOG4CPLUS_TEXT (' ')
        << event.getTimestamp ().getFormattedTime (remoteTimeFormat, true) 
        // HOSTNAME
        << LOG4CPLUS_TEXT (' ') << helpers::getHostname (true)
        // APP-NAME
        << LOG4CPLUS_TEXT (' ') << ident
        // PROCID
        << LOG4CPLUS_TEXT (' ') << internal::get_process_id ()
        // MSGID
        << LOG4CPLUS_TEXT (' ') << event.getLoggerName ()
        // STRUCTURED-DATA
        // no structured data, it could be whole MDC
        << LOG4CPLUS_TEXT (" - ");
    
    // MSG
    layout->formatAndAppend (appender_sp.oss, event);

    std::string str (LOG4CPLUS_TSTRING_TO_STRING (appender_sp.oss.str ()));
    std::size_t const str_size = str.size ();
    helpers::SocketBuffer sbuf (str_size);
    sbuf.setSize (str_size);
    std::memcpy (sbuf.getBuffer (), str.c_str (), str_size);

    bool ret = syslogSocket.write (sbuf);
    if (! ret)
    {
        helpers::getLogLog ().warn (
            LOG4CPLUS_TEXT ("SysLogAppender::appendRemote")
            LOG4CPLUS_TEXT ("- socket write failed"));
        syslogSocket = helpers::Socket (host, port, true);
    }
}


} // namespace log4cplus


#endif // defined(LOG4CPLUS_HAVE_SYSLOG_H)

