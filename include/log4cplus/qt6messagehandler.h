// -*- C++ -*-
// Module:  Log4cplus
// File:    qt6messagehandler.h
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


//

/** @file */

#ifndef LOG4CPLUS_QT6MESSAGEHANDLER_H
#define LOG4CPLUS_QT6MESSAGEHANDLER_H

#include <log4cplus/config.hxx>

#if defined (LOG4CPLUS_HAVE_PRAGMA_ONCE)
#pragma once
#endif

#include <QtCore>
#include <type_traits>

#if defined (_WIN32)
  #if defined (log4cplusqt6debugappender_EXPORTS) \
      || defined (log4cplusqt6debugappenderU_EXPORTS) \
      || (defined (DLL_EXPORT) && defined (INSIDE_LOG4CPLUS_QT6DEBUGAPPENDER))
    #undef LOG4CPLUS_QT6DEBUGAPPENDER_BUILD_DLL
    #define LOG4CPLUS_QT6DEBUGAPPENDER_BUILD_DLL
  #endif
  #if defined (LOG4CPLUS_QT6DEBUGAPPENDER_BUILD_DLL)
    #if defined (INSIDE_LOG4CPLUS_QT6DEBUGAPPENDER)
      #define LOG4CPLUS_QT6DEBUGAPPENDER_EXPORT __declspec(dllexport)
    #else
      #define LOG4CPLUS_QT6DEBUGAPPENDER_EXPORT __declspec(dllimport)
    #endif
  #else
    #define LOG4CPLUS_QT6DEBUGAPPENDER_EXPORT
  #endif
#else
  #if defined (INSIDE_LOG4CPLUS_QT6DEBUGAPPENDER)
    #define LOG4CPLUS_QT6DEBUGAPPENDER_EXPORT LOG4CPLUS_DECLSPEC_EXPORT
  #else
    #define LOG4CPLUS_QT6DEBUGAPPENDER_EXPORT LOG4CPLUS_DECLSPEC_IMPORT
  #endif // defined (INSIDE_LOG4CPLUS_QT6DEBUGAPPENDER)
#endif // !_WIN32


namespace log4cplus
{

using QtMessageHandlerType = std::remove_pointer_t<QtMessageHandler>;

/**
 * qt6_message_handler is a global message handler for Qt6 that logs messages
 * using log4cplus.
 *
 * @since 3.0.0
 *
 * @note This handler will call `std::_Exit(EXIT_FAILURE)` for `QtFatalMsg`
 * messages.
 * @note This handler is intended to be used with Qt6 applications.
 * @note This is a funtion declaration, not a class.
 *
 * Example usage:
 *
 *     #include <QtApplication>
 *     #include <log4cplus/qt6messagehandler.h>
 *     #include <log4cplus/initializer.h>
 *
 *     int main (int argc, char *argv[]) {
 *       log4cplus::Initializer initializer;
 *       QApplication app (argc, argv);
 *       auto originalHandler = qInstallMessageHandler (qt6_message_handler);
 *       // Your application code here
 *       // ...
 *     }
 */
LOG4CPLUS_QT6DEBUGAPPENDER_EXPORT
void
qt6_message_handler (QtMsgType type, QMessageLogContext const & qt_log_context,
    QString const & message);

static_assert (std::is_same_v<decltype(qt6_message_handler),
    std::remove_pointer_t<QtMessageHandler>>,
    "qt6_message_handler must have the same signature as QtMessageHandlerType.");

} // namespace log4cplus


#endif // LOG4CPLUS_QT6MESSAGEHANDLER_H
