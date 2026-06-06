// -*- C++ -*-
// Module:  Log4CPLUS
// File:    opentelemetryappender.h
// Created: 6/2026
// Author:  Vaclav Haisman
//
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

#ifndef LOG4CPLUS_OPENTELEMETRY_APPENDER_HEADER_
#define LOG4CPLUS_OPENTELEMETRY_APPENDER_HEADER_

#include <log4cplus/config.hxx>

#if defined (LOG4CPLUS_HAVE_PRAGMA_ONCE)
#pragma once
#endif

#include <log4cplus/appender.h>

#include <memory>

#if defined (_WIN32)
  #if defined (LOG4CPLUS_OPENTELEMETRYAPPENDER_BUILD_DLL)
    #if defined (INSIDE_LOG4CPLUS_OPENTELEMETRYAPPENDER)
      #define LOG4CPLUS_OPENTELEMETRYAPPENDER_EXPORT __declspec(dllexport)
    #else
      #define LOG4CPLUS_OPENTELEMETRYAPPENDER_EXPORT __declspec(dllimport)
    #endif
  #else
    #define LOG4CPLUS_OPENTELEMETRYAPPENDER_EXPORT
  #endif
#else
  #if defined (INSIDE_LOG4CPLUS_OPENTELEMETRYAPPENDER)
    #define LOG4CPLUS_OPENTELEMETRYAPPENDER_EXPORT LOG4CPLUS_DECLSPEC_EXPORT
  #else
    #define LOG4CPLUS_OPENTELEMETRYAPPENDER_EXPORT LOG4CPLUS_DECLSPEC_IMPORT
  #endif
#endif


namespace log4cplus
{

    /**
     * Appends log events to OpenTelemetry C++ Logs.
     *
     * This appender is provided by the optional OpenTelemetry companion
     * library. The core log4cplus library does not depend on OpenTelemetry C++
     * and does not register this appender by default. Link or load the companion
     * library and call registerAppender() before configuring by class name.
     *
     * <h3>Properties</h3>
     * <dl>
     * <dt><tt>ProviderMode</tt></dt>
     * <dd><tt>Global</tt> (default) uses the process-global OpenTelemetry
     * LoggerProvider. <tt>Otlp</tt> creates an appender-owned OTLP logs
     * pipeline when log4cplus was built with OTLP appender support.</dd>
     *
     * <dt><tt>InstrumentationName</tt></dt>
     * <dd>Instrumentation scope name passed to OpenTelemetry. Defaults to
     * <tt>log4cplus</tt>.</dd>
     *
     * <dt><tt>InstrumentationVersion</tt></dt>
     * <dd>Optional instrumentation scope version.</dd>
     *
     * <dt><tt>IncludeLocation</tt></dt>
     * <dd>Boolean controlling source file, line, and function attributes.
     * Defaults to <tt>true</tt>.</dd>
     *
     * <dt><tt>IncludeMDC</tt></dt>
     * <dd>Boolean controlling MDC attributes under
     * <tt>log4cplus.mdc.</tt>. Defaults to <tt>true</tt>.</dd>
     *
     * <dt><tt>IncludeNDC</tt></dt>
     * <dd>Boolean controlling the <tt>log4cplus.ndc</tt> attribute. Defaults
     * to <tt>true</tt>.</dd>
     *
     * <dt><tt>UseLayout</tt></dt>
     * <dd>Boolean controlling whether the OpenTelemetry log body uses this
     * appender's layout output. Defaults to <tt>false</tt>; by default the
     * original event message is used as the structured log body.</dd>
     *
     * <dt><tt>OtlpProtocol</tt></dt>
     * <dd>For <tt>ProviderMode=Otlp</tt>, selects <tt>grpc</tt> or
     * <tt>http/protobuf</tt>, subject to the OpenTelemetry C++ targets linked
     * into log4cplus. Defaults to <tt>grpc</tt> when available, otherwise
     * <tt>http/protobuf</tt>.</dd>
     *
     * <dt><tt>OtlpEndpoint</tt></dt>
     * <dd>Optional OTLP endpoint. Empty uses the OpenTelemetry C++ exporter
     * default and any standard OTEL_* environment variable handling provided
     * by the SDK.</dd>
     *
     * <dt><tt>OtlpHeaders</tt></dt>
     * <dd>Optional comma-separated <tt>key=value</tt> header list for exporters
     * that support it.</dd>
     *
     * <dt><tt>OtlpTimeout</tt></dt>
     * <dd>Optional exporter timeout in milliseconds.</dd>
     *
     * <dt><tt>OtlpUseSslCredentials</tt></dt>
     * <dd>Optional boolean TLS hint for the gRPC exporter when supported by
     * the installed OpenTelemetry C++ version.</dd>
     *
     * <dt><tt>ResourceAttributes</tt></dt>
     * <dd>For appender-owned OTLP providers, optional comma-separated
     * <tt>key=value</tt> resource attributes.</dd>
     *
     * <dt><tt>ServiceName</tt></dt>
     * <dd>Convenience property mapped to the resource attribute
     * <tt>service.name</tt> for appender-owned OTLP providers.</dd>
     * </dl>
     */
    class LOG4CPLUS_OPENTELEMETRYAPPENDER_EXPORT OpenTelemetryAppender
        : public Appender
    {
    public:
        OpenTelemetryAppender();
        OpenTelemetryAppender(
            const log4cplus::helpers::Properties & properties);
        OpenTelemetryAppender(const OpenTelemetryAppender&) = delete;
        OpenTelemetryAppender& operator=(const OpenTelemetryAppender&) = delete;

        virtual ~OpenTelemetryAppender();

        virtual void close() override;

        static void registerAppender ();

    protected:
        virtual void append(const spi::InternalLoggingEvent& event) override;

    private:
        struct Data;
        std::unique_ptr<Data> data;
    };

} // end namespace log4cplus


#endif // LOG4CPLUS_OPENTELEMETRY_APPENDER_HEADER_
