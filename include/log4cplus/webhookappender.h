// -*- C++ -*-
// Module:  Log4CPLUS
// File:    webhookappender.h
// Created: 2025
// Author:  Custom Implementation
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

#ifndef LOG4CPLUS_WEBHOOK_APPENDER_HEADER_
#define LOG4CPLUS_WEBHOOK_APPENDER_HEADER_

#include <log4cplus/config.hxx>

#if defined (LOG4CPLUS_HAVE_PRAGMA_ONCE)
#pragma once
#endif

#include <log4cplus/appender.h>
#include <log4cplus/tstring.h>
#include <vector>
#include <memory>


namespace log4cplus
{

    /**
     * WebhookAppender sends log events to a remote webhook via HTTP POST.
     *
     * The appender only sends log events that meet the following criteria:
     * - Log level is ERROR or higher
     * - The log message contains one of the configured keywords
     *
     * <h3>Properties</h3>
     * <dl>
     *
     * <dt><tt>WebhookURL</tt></dt>
     * <dd>The URL of the webhook endpoint to send POST requests to.</dd>
     *
     * <dt><tt>Keywords</tt></dt>
     * <dd>A comma-separated list of keywords. If any keyword is found in the
     * log message, the event will be sent to the webhook.</dd>
     *
     * <dt><tt>Timeout</tt></dt>
     * <dd>HTTP request timeout in seconds. Default is 30 seconds.</dd>
     *
     * <dt><tt>Headers</tt></dt>
     * <dd>Additional HTTP headers to include in the request. Format: "Header1:Value1,Header2:Value2"</dd>
     *
     * </dl>
     */
    class LOG4CPLUS_EXPORT WebhookAppender
        : public Appender
    {
    public:
      // Ctors
        WebhookAppender(const log4cplus::tstring& webhookURL,
            const log4cplus::tstring& keywords = tstring(),
            int timeout = 30);
        WebhookAppender(const log4cplus::helpers::Properties & properties);
        // Disallow copying of instances of this class
        WebhookAppender(const WebhookAppender&) = delete;
        WebhookAppender& operator=(const WebhookAppender&) = delete;

      // Dtor
        ~WebhookAppender();

      // Methods
        virtual void close() override;

    protected:
        virtual void append(const spi::InternalLoggingEvent& event) override;

      // Data
        log4cplus::tstring webhookURL;
        std::vector<log4cplus::tstring> keywords;
        int timeout;
        std::vector<std::pair<log4cplus::tstring, log4cplus::tstring>> headers;

    private:
        bool shouldSend(const spi::InternalLoggingEvent& event) const;
        bool containsKeyword(const log4cplus::tstring& message) const;
        bool sendWebhook(const spi::InternalLoggingEvent& event) const;
        log4cplus::tstring formatJSON(const spi::InternalLoggingEvent& event) const;
        void parseHeaders(const log4cplus::tstring& headersStr);
        void parseKeywords(const log4cplus::tstring& keywordsStr);
    };

} // end namespace log4cplus

#endif // LOG4CPLUS_WEBHOOK_APPENDER_HEADER_
