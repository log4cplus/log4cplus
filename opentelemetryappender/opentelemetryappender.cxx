// -*- C++ -*-
// Module:  Log4CPLUS
// File:    opentelemetryappender.cxx
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

#include <log4cplus/opentelemetryappender.h>

#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/spi/factory.h>
#include <log4cplus/spi/loggingevent.h>

#include <opentelemetry/common/attribute_value.h>
#include <opentelemetry/common/timestamp.h>
#include <opentelemetry/logs/log_record.h>
#include <opentelemetry/logs/logger.h>
#include <opentelemetry/logs/logger_provider.h>
#include <opentelemetry/logs/provider.h>
#include <opentelemetry/logs/severity.h>
#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/nostd/string_view.h>

#if defined (LOG4CPLUS_WITH_OPENTELEMETRY_OTLP)
#include <opentelemetry/sdk/logs/logger_provider.h>
#include <opentelemetry/sdk/logs/logger_provider_factory.h>
#include <opentelemetry/sdk/logs/simple_log_record_processor_factory.h>
#include <opentelemetry/sdk/resource/resource.h>
#if defined (LOG4CPLUS_WITH_OPENTELEMETRY_OTLP_GRPC)
#include <opentelemetry/exporters/otlp/otlp_grpc_log_record_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_grpc_log_record_exporter_options.h>
#endif
#if defined (LOG4CPLUS_WITH_OPENTELEMETRY_OTLP_HTTP)
#include <opentelemetry/exporters/otlp/otlp_http_log_record_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_http_log_record_exporter_options.h>
#endif
#endif

#include <algorithm>
#include <chrono>
#include <cctype>
#include <exception>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace log4cplus
{

namespace
{

namespace otel_logs = opentelemetry::logs;
namespace otel_common = opentelemetry::common;

std::string
narrow (log4cplus::tstring const & str)
{
    return LOG4CPLUS_TSTRING_TO_STRING (str);
}

std::string
trim (std::string str)
{
    auto const not_space = [] (unsigned char ch) { return ! std::isspace (ch); };
    str.erase (str.begin (), std::find_if (str.begin (), str.end (), not_space));
    str.erase (std::find_if (str.rbegin (), str.rend (), not_space).base (), str.end ());
    return str;
}

std::string
lower (std::string str)
{
    std::transform (str.begin (), str.end (), str.begin (),
        [] (unsigned char ch) { return static_cast<char> (std::tolower (ch)); });
    return str;
}

std::vector<std::pair<std::string, std::string> >
parseKeyValueList (std::string const & text)
{
    std::vector<std::pair<std::string, std::string> > result;
    std::string::size_type first = 0;

    while (first <= text.size ())
    {
        std::string::size_type const comma = text.find (',', first);
        std::string item = text.substr (first,
            comma == std::string::npos ? std::string::npos : comma - first);
        item = trim (item);
        if (! item.empty ())
        {
            std::string::size_type const equals = item.find ('=');
            if (equals == std::string::npos)
                helpers::getLogLog ().warn (LOG4CPLUS_TEXT (
                    "OpenTelemetryAppender: ignoring key/value item without '='"));
            else
            {
                std::string key = trim (item.substr (0, equals));
                std::string value = trim (item.substr (equals + 1));
                if (! key.empty ())
                    result.emplace_back (std::move (key), std::move (value));
            }
        }

        if (comma == std::string::npos)
            break;
        first = comma + 1;
    }

    return result;
}

otel_logs::Severity
severity (LogLevel ll)
{
    if (ll >= FATAL_LOG_LEVEL)
        return otel_logs::Severity::kFatal;
    else if (ll >= ERROR_LOG_LEVEL)
        return otel_logs::Severity::kError;
    else if (ll >= WARN_LOG_LEVEL)
        return otel_logs::Severity::kWarn;
    else if (ll >= INFO_LOG_LEVEL)
        return otel_logs::Severity::kInfo;
    else if (ll >= DEBUG_LOG_LEVEL)
        return otel_logs::Severity::kDebug;
    else
        return otel_logs::Severity::kTrace;
}

otel_common::SystemTimestamp
systemTimestamp (helpers::Time const & timestamp)
{
    return otel_common::SystemTimestamp (
        std::chrono::system_clock::time_point (
            std::chrono::duration_cast<std::chrono::system_clock::duration> (
                timestamp.time_since_epoch ())));
}

void
setStringAttribute (otel_logs::LogRecord & record, char const * key,
    log4cplus::tstring const & value)
{
    if (! value.empty ())
    {
        std::string str = narrow (value);
        record.SetAttribute (key, otel_common::AttributeValue (str));
    }
}

} // namespace

struct OpenTelemetryAppender::Data
{
    enum ProviderMode
    {
        Global,
        Otlp
    };

    ProviderMode providerMode = Global;
    std::string instrumentationName = "log4cplus";
    std::string instrumentationVersion;
    bool includeLocation = true;
    bool includeMDC = true;
    bool includeNDC = true;
    bool useLayout = false;
    bool closed = false;
    bool appendWarningIssued = false;
    opentelemetry::nostd::shared_ptr<otel_logs::LoggerProvider> provider;
    opentelemetry::nostd::shared_ptr<otel_logs::Logger> logger;

#if defined (LOG4CPLUS_WITH_OPENTELEMETRY_OTLP)
    std::shared_ptr<opentelemetry::sdk::logs::LoggerProvider> ownedProvider;
#endif
};

OpenTelemetryAppender::OpenTelemetryAppender()
    : Appender()
    , data (new Data)
{
    data->provider = otel_logs::Provider::GetLoggerProvider ();
    data->logger = data->provider->GetLogger (data->instrumentationName,
        data->instrumentationName, data->instrumentationVersion);
}

OpenTelemetryAppender::OpenTelemetryAppender(
    const log4cplus::helpers::Properties & properties)
    : Appender(properties)
    , data (new Data)
{
    tstring value;
    if (properties.getString (value, LOG4CPLUS_TEXT ("ProviderMode")))
    {
        std::string const mode = lower (narrow (value));
        if (mode == "global")
            data->providerMode = Data::Global;
        else if (mode == "otlp")
            data->providerMode = Data::Otlp;
        else
            helpers::getLogLog ().warn (LOG4CPLUS_TEXT (
                "OpenTelemetryAppender: unsupported ProviderMode; using Global"));
    }

    if (properties.getString (value, LOG4CPLUS_TEXT ("InstrumentationName")))
        data->instrumentationName = narrow (value);
    if (data->instrumentationName.empty ())
        data->instrumentationName = "log4cplus";

    if (properties.getString (value, LOG4CPLUS_TEXT ("InstrumentationVersion")))
        data->instrumentationVersion = narrow (value);

    properties.getBool (data->includeLocation,
        LOG4CPLUS_TEXT ("IncludeLocation"));
    properties.getBool (data->includeMDC, LOG4CPLUS_TEXT ("IncludeMDC"));
    properties.getBool (data->includeNDC, LOG4CPLUS_TEXT ("IncludeNDC"));
    properties.getBool (data->useLayout, LOG4CPLUS_TEXT ("UseLayout"));

    try
    {
        if (data->providerMode == Data::Global)
            data->provider = otel_logs::Provider::GetLoggerProvider ();
        else
        {
#if defined (LOG4CPLUS_WITH_OPENTELEMETRY_OTLP)
            std::string protocol;
            if (properties.getString (value, LOG4CPLUS_TEXT ("OtlpProtocol")))
                protocol = lower (narrow (value));

            if (protocol.empty ())
            {
#if defined (LOG4CPLUS_WITH_OPENTELEMETRY_OTLP_GRPC)
                protocol = "grpc";
#else
                protocol = "http/protobuf";
#endif
            }

            std::unique_ptr<opentelemetry::sdk::logs::LogRecordExporter> exporter;

#if defined (LOG4CPLUS_WITH_OPENTELEMETRY_OTLP_GRPC)
            if (protocol == "grpc")
            {
                opentelemetry::exporter::otlp::OtlpGrpcLogRecordExporterOptions opts;
                if (properties.getString (value, LOG4CPLUS_TEXT ("OtlpEndpoint")))
                    opts.endpoint = narrow (value);
                int timeout = 0;
                if (properties.getInt (timeout, LOG4CPLUS_TEXT ("OtlpTimeout")))
                    opts.timeout = std::chrono::milliseconds (timeout);
                if (properties.getString (value, LOG4CPLUS_TEXT ("OtlpHeaders")))
                    for (auto const & header : parseKeyValueList (narrow (value)))
                        opts.metadata.emplace (header.first, header.second);
                bool useSslCredentials = false;
                if (properties.getBool (useSslCredentials,
                        LOG4CPLUS_TEXT ("OtlpUseSslCredentials")))
                    opts.use_ssl_credentials = useSslCredentials;
                exporter = opentelemetry::exporter::otlp::
                    OtlpGrpcLogRecordExporterFactory::Create (opts);
            }
#endif

#if defined (LOG4CPLUS_WITH_OPENTELEMETRY_OTLP_HTTP)
            if (! exporter
                && (protocol == "http/protobuf" || protocol == "http"))
            {
                opentelemetry::exporter::otlp::OtlpHttpLogRecordExporterOptions opts;
                if (properties.getString (value, LOG4CPLUS_TEXT ("OtlpEndpoint")))
                    opts.url = narrow (value);
                int timeout = 0;
                if (properties.getInt (timeout, LOG4CPLUS_TEXT ("OtlpTimeout")))
                    opts.timeout = std::chrono::milliseconds (timeout);
                if (properties.getString (value, LOG4CPLUS_TEXT ("OtlpHeaders")))
                    for (auto const & header : parseKeyValueList (narrow (value)))
                        opts.http_headers.emplace (header.first, header.second);
                exporter = opentelemetry::exporter::otlp::
                    OtlpHttpLogRecordExporterFactory::Create (opts);
            }
#endif

            if (! exporter)
            {
                helpers::getLogLog ().warn (LOG4CPLUS_TEXT (
                    "OpenTelemetryAppender: requested OTLP protocol is not built; using Global provider"));
                data->providerMode = Data::Global;
                data->provider = otel_logs::Provider::GetLoggerProvider ();
            }
            else
            {
                auto processor = opentelemetry::sdk::logs::
                    SimpleLogRecordProcessorFactory::Create (std::move (exporter));

                opentelemetry::sdk::resource::ResourceAttributes attrs;
                if (properties.getString (value,
                        LOG4CPLUS_TEXT ("ResourceAttributes")))
                    for (auto & item : parseKeyValueList (narrow (value)))
                        attrs[item.first] = item.second;
                if (properties.getString (value, LOG4CPLUS_TEXT ("ServiceName"))
                    && ! value.empty ())
                    attrs["service.name"] = narrow (value);

                auto resource = opentelemetry::sdk::resource::Resource::Create (attrs);
                std::unique_ptr<opentelemetry::sdk::logs::LoggerProvider> owned
                    = opentelemetry::sdk::logs::LoggerProviderFactory::Create (
                        std::move (processor), resource);
                data->ownedProvider.reset (owned.release ());
                data->provider = opentelemetry::nostd::shared_ptr<otel_logs::LoggerProvider> (
                    std::static_pointer_cast<otel_logs::LoggerProvider> (
                        data->ownedProvider));
            }
#else
            helpers::getLogLog ().warn (LOG4CPLUS_TEXT (
                "OpenTelemetryAppender: ProviderMode=Otlp requires log4cplus to be built with OTLP appender support; using Global provider"));
            data->providerMode = Data::Global;
            data->provider = otel_logs::Provider::GetLoggerProvider ();
#endif
        }

        data->logger = data->provider->GetLogger (data->instrumentationName,
            data->instrumentationName, data->instrumentationVersion);
    }
    catch (std::exception const &)
    {
        helpers::getLogLog ().warn (LOG4CPLUS_TEXT (
            "OpenTelemetryAppender: failed to initialize OpenTelemetry provider"));
        data->provider = otel_logs::Provider::GetLoggerProvider ();
        data->logger = data->provider->GetLogger (data->instrumentationName,
            data->instrumentationName, data->instrumentationVersion);
    }
}

OpenTelemetryAppender::~OpenTelemetryAppender()
{
    destructorImpl();
}

void
OpenTelemetryAppender::close()
{
    if (data->closed)
        return;

    data->closed = true;
#if defined (LOG4CPLUS_WITH_OPENTELEMETRY_OTLP)
    if (data->ownedProvider)
    {
        data->ownedProvider->ForceFlush (std::chrono::microseconds::max ());
        data->ownedProvider->Shutdown (std::chrono::microseconds::max ());
    }
#endif
    data->logger = nullptr;
    data->provider = nullptr;
}

void
OpenTelemetryAppender::append(const spi::InternalLoggingEvent& event)
{
    if (data->closed || ! data->logger)
        return;

    try
    {
        auto record = data->logger->CreateLogRecord ();
        if (! record)
            return;

        record->SetTimestamp (systemTimestamp (event.getTimestamp ()));
        record->SetObservedTimestamp (otel_common::SystemTimestamp (
            std::chrono::system_clock::now ()));
        record->SetSeverity (severity (event.getLogLevel ()));

        // By default we preserve the original structured log message as the
        // OpenTelemetry body. UseLayout is opt-in for applications that want
        // the body to match existing text appenders exactly.
        std::string body = data->useLayout
            ? narrow (formatEvent (event))
            : narrow (event.getMessage ());
        record->SetBody (otel_common::AttributeValue (body));

        setStringAttribute (*record, "log4cplus.logger_name",
            event.getLoggerName ());
        setStringAttribute (*record, "thread.name", event.getThread ());
        setStringAttribute (*record, "thread.id", event.getThread2 ());

        if (data->includeLocation)
        {
            setStringAttribute (*record, "code.filepath", event.getFile ());
            if (event.getLine () >= 0)
                record->SetAttribute ("code.lineno",
                    otel_common::AttributeValue (event.getLine ()));
            setStringAttribute (*record, "code.function", event.getFunction ());
        }

        if (data->includeNDC)
            setStringAttribute (*record, "log4cplus.ndc", event.getNDC ());

        if (data->includeMDC)
        {
            for (auto const & item : event.getMDCCopy ())
            {
                std::string key = "log4cplus.mdc." + narrow (item.first);
                std::string value = narrow (item.second);
                record->SetAttribute (opentelemetry::nostd::string_view (key),
                    otel_common::AttributeValue (value));
            }
        }

        data->logger->EmitLogRecord (std::move (record));
    }
    catch (...)
    {
        if (! data->appendWarningIssued)
        {
            data->appendWarningIssued = true;
            helpers::getLogLog ().warn (LOG4CPLUS_TEXT (
                "OpenTelemetryAppender: failed to emit OpenTelemetry log record"));
        }
    }
}

void
OpenTelemetryAppender::registerAppender ()
{
    log4cplus::spi::AppenderFactoryRegistry & reg
        = log4cplus::spi::getAppenderFactoryRegistry ();
    LOG4CPLUS_REG_APPENDER (reg, OpenTelemetryAppender);
}

} // namespace log4cplus
