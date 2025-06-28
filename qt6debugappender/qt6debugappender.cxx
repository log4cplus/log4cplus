// Module:  Log4cplus
// File:    qt6debugappender.cxx
// Created: 6/2025
// Author:  Vaclav Haisman
//
//
//  Copyright (C) 2025, Vaclav Haisman. All rights reserved.
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

#include <log4cplus/config.hxx>
#include <log4cplus/qt6debugappender.h>
#include <log4cplus/qt6messagehandler.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/spi/factory.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/logger.h>
#include <sstream>
#include <iomanip>
#include <QtGlobal>
#include <QString>
#include <log4cplus/config/windowsh-inc.h>


// Forward Declarations
namespace log4cplus
{


Qt6DebugAppender::Qt6DebugAppender ()
    : Appender ()
{ }


Qt6DebugAppender::Qt6DebugAppender (helpers::Properties const & props)
    : Appender (props)
{ }


Qt6DebugAppender::~Qt6DebugAppender ()
{
    destructorImpl ();
}


void
Qt6DebugAppender::close ()
{ }


void
Qt6DebugAppender::append (spi::InternalLoggingEvent const & ev)
{
    // TODO: Expose log4cplus' internal TLS to use here.
    tostringstream oss;
    layout->formatAndAppend(oss, ev);

    LogLevel const ll = ev.getLogLevel ();
    std::string const & file = LOG4CPLUS_TSTRING_TO_STRING (ev.getFile ());
    std::string const & func = LOG4CPLUS_TSTRING_TO_STRING (ev.getFunction ());
    std::string const & logger
        = LOG4CPLUS_TSTRING_TO_STRING (ev.getLoggerName ());
    int const line = ev.getLine ();

    QMessageLogger qlogger (file.c_str (), line, func.c_str (),
        logger.c_str ());
    void (QMessageLogger:: * log_func) (const char *, ...) const = 0;

    if (ll >= ERROR_LOG_LEVEL)
        log_func = &QMessageLogger::critical;
    else if (ll >= WARN_LOG_LEVEL)
        log_func = &QMessageLogger::warning;
    else if (ll >= INFO_LOG_LEVEL)
        log_func = &QMessageLogger::info;
    else
        log_func = &QMessageLogger::debug;

    (qlogger.*log_func) ("%s",
        LOG4CPLUS_TSTRING_TO_STRING (oss.str ()).c_str ());
}


void
Qt6DebugAppender::registerAppender ()
{
    log4cplus::spi::AppenderFactoryRegistry & reg
        = log4cplus::spi::getAppenderFactoryRegistry ();
    LOG4CPLUS_REG_APPENDER (reg, Qt6DebugAppender);
}


//
// qt6_message_handler definition
//

namespace
{

/// @brief Convert Qt message type to log4cplus LogLevel.
static inline
LogLevel
qt_log_level_to_log4cplus (QtMsgType const type)
{
    switch (type) {
    case QtDebugMsg:
        return DEBUG_LOG_LEVEL;
    case QtInfoMsg:
        return INFO_LOG_LEVEL;
    case QtWarningMsg:
        return WARN_LOG_LEVEL;
    case QtCriticalMsg:
        return ERROR_LOG_LEVEL;
    case QtFatalMsg:
        return FATAL_LOG_LEVEL;
    default:
        return NOT_SET_LOG_LEVEL; // Should not happen.
    }

}

/// @brief String conversion utility for Qt strings to log4cplus tstring.
template <typename T>
static inline
std::basic_string<T>
qt_string_to_tstring (QString const & str)
{
    if constexpr (std::is_same_v<T, char>) {
        // If tchar is char, we can use QString's toStdString directly.
        return str.toStdString ();
    } else if constexpr (std::is_same_v<T, wchar_t>) {
        // If tchar is wchar_t, we need to convert QString to std::wstring.
        return str.toStdWString ();
    } else {
        static_assert(false,
            "Unsupported tchar type. Only char and wchar_t are supported.");
    }
}

} // anonymous namespace


void
qt6_message_handler (QtMsgType type, QMessageLogContext const & qt_log_context, QString const & message)
{
    // Convert the Qt message to a log4cplus logging event.
    spi::InternalLoggingEvent ev {
        LOG4CPLUS_TEXT ("QtCore"),
        qt_log_level_to_log4cplus (type),
        qt_string_to_tstring<log4cplus::tchar> (message),
        qt_log_context.file ? qt_log_context.file : "<QtCore>",
        qt_log_context.line,
        qt_log_context.function
    };

    Logger::getInstance(LOG4CPLUS_TEXT("QtCore"))
        .log (ev);
}

static_assert (std::is_same_v<decltype(qt6_message_handler), QtMessageHandlerType>,
    "qt6_message_handler must have the same signature as QtMessageHandlerType.");


} // namespace log4cplus


#if defined (_WIN32)
extern "C"
BOOL WINAPI DllMain(LOG4CPLUS_DLLMAIN_HINSTANCE,  // handle to DLL module
                    DWORD fdwReason,     // reason for calling function
                    LPVOID)  // reserved
{
    // Perform actions based on the reason for calling.
    switch( fdwReason )
    {
    case DLL_PROCESS_ATTACH:
    {
        // We cannot do this here because it causes the thread to deadlock
        // when compiled with Visual Studio due to use of C++11 threading
        // facilities.

        //log4cplus::Qt6DebugAppender::registerAppender ();
        break;
    }

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

#endif // defined (_WIN32)
