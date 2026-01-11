// Module:  Log4CPLUS
// File:    webhookappender.cxx
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

#include <log4cplus/webhookappender.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/helpers/stringhelper.h>
#include <sstream>
#include <algorithm>
#include <cctype>

#if defined(_WIN32)
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#endif

namespace log4cplus {

namespace {

static tstring trim(const tstring& str)
{
    if (str.empty())
        return str;

    size_t start = 0;
    size_t end = str.length();

    while (start < end && std::isspace(static_cast<unsigned char>(str[start])))
        ++start;

    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1])))
        --end;

    return str.substr(start, end - start);
}

}

//////////////////////////////////////////////////////////////////////////////
// WebhookAppender ctors and dtor
//////////////////////////////////////////////////////////////////////////////

WebhookAppender::WebhookAppender(const tstring& webhookURL_,
    const tstring& keywords_, int timeout_)
    : webhookURL(webhookURL_)
    , timeout(timeout_)
{
    parseKeywords(keywords_);
}


WebhookAppender::WebhookAppender(const helpers::Properties & properties)
 : Appender(properties)
{
    webhookURL = properties.getProperty(LOG4CPLUS_TEXT("WebhookURL"));
    tstring keywordsStr = properties.getProperty(LOG4CPLUS_TEXT("Keywords"));
    parseKeywords(keywordsStr);

    properties.getInt(timeout, LOG4CPLUS_TEXT("Timeout"));
    if (timeout <= 0)
        timeout = 30;

    tstring headersStr = properties.getProperty(LOG4CPLUS_TEXT("Headers"));
    parseHeaders(headersStr);
}


WebhookAppender::~WebhookAppender()
{
    destructorImpl();
}



//////////////////////////////////////////////////////////////////////////////
// WebhookAppender public methods
//////////////////////////////////////////////////////////////////////////////

void
WebhookAppender::close()
{
    helpers::getLogLog().debug(
        LOG4CPLUS_TEXT("Entering WebhookAppender::close()..."));
    closed = true;
}



//////////////////////////////////////////////////////////////////////////////
// WebhookAppender protected methods
//////////////////////////////////////////////////////////////////////////////

void
WebhookAppender::append(const spi::InternalLoggingEvent& event)
{
    if (closed)
        return;

    if (!shouldSend(event))
        return;

    try
    {
        sendWebhook(event);
    }
    catch (const std::exception& e)
    {
        helpers::getLogLog().error(
            LOG4CPLUS_TEXT("WebhookAppender::append() - Exception: ")
            + LOG4CPLUS_C_STR_TO_TSTRING(e.what()));
    }
    catch (...)
    {
        helpers::getLogLog().error(
            LOG4CPLUS_TEXT("WebhookAppender::append() - Unknown exception"));
    }
}


//////////////////////////////////////////////////////////////////////////////
// WebhookAppender private methods
//////////////////////////////////////////////////////////////////////////////

bool
WebhookAppender::shouldSend(const spi::InternalLoggingEvent& event) const
{
    if (event.getLogLevel() < ERROR_LOG_LEVEL)
        return false;

    if (keywords.empty())
        return true;

    tstring message = event.getMessage();
    return containsKeyword(message);
}


bool
WebhookAppender::containsKeyword(const tstring& message) const
{
    tstring lowerMessage = helpers::toLower(message);
    
    for (const auto& keyword : keywords)
    {
        tstring lowerKeyword = helpers::toLower(keyword);
        if (lowerMessage.find(lowerKeyword) != tstring::npos)
            return true;
    }
    
    return false;
}


bool
WebhookAppender::sendWebhook(const spi::InternalLoggingEvent& event) const
{
#if defined(_WIN32)
    tstring jsonStr = formatJSON(event);

    std::string urlA;
    std::string jsonA;
    std::string headersA;

#ifdef UNICODE
    urlA = helpers::tostring(webhookURL);
    jsonA = helpers::tostring(jsonStr);
#else
    urlA = webhookURL;
    jsonA = jsonStr;
#endif

    HINTERNET hSession = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    bool success = false;

    try
    {
        hSession = WinHttpOpen(
            L"Log4CPlus WebhookAppender/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS,
            0);

        if (!hSession)
            throw std::runtime_error("WinHttpOpen failed");

        URL_COMPONENTS urlComponents = {0};
        urlComponents.dwStructSize = sizeof(URL_COMPONENTS);
        urlComponents.dwSchemeLength = (DWORD)-1;
        urlComponents.dwHostNameLength = (DWORD)-1;
        urlComponents.dwUrlPathLength = (DWORD)-1;
        urlComponents.dwExtraInfoLength = (DWORD)-1;

        std::wstring urlW(urlA.begin(), urlA.end());
        if (!WinHttpCrackUrl(urlW.c_str(), 0, 0, &urlComponents))
            throw std::runtime_error("WinHttpCrackUrl failed");

        std::wstring hostName(urlComponents.lpszHostName, urlComponents.dwHostNameLength);
        std::wstring urlPath(urlComponents.lpszUrlPath, urlComponents.dwUrlPathLength);
        if (urlComponents.lpszExtraInfo)
            urlPath += std::wstring(urlComponents.lpszExtraInfo, urlComponents.dwExtraInfoLength);

        INTERNET_PORT port = urlComponents.nPort;
        if (port == 0)
        {
            port = (urlComponents.nScheme == INTERNET_SCHEME_HTTPS) ? 443 : 80;
        }

        hConnect = WinHttpConnect(
            hSession,
            hostName.c_str(),
            port,
            0);

        if (!hConnect)
            throw std::runtime_error("WinHttpConnect failed");

        DWORD flags = (urlComponents.nScheme == INTERNET_SCHEME_HTTPS) 
            ? WINHTTP_FLAG_SECURE 
            : 0;

        hRequest = WinHttpOpenRequest(
            hConnect,
            L"POST",
            urlPath.c_str(),
            NULL,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            flags);

        if (!hRequest)
            throw std::runtime_error("WinHttpOpenRequest failed");

        DWORD timeoutMs = timeout * 1000;
        WinHttpSetTimeouts(hRequest, timeoutMs, timeoutMs, timeoutMs, timeoutMs);

        for (const auto& header : headers)
        {
            std::wstring headerStr;
#ifdef UNICODE
            headerStr = header.first + L": " + header.second;
#else
            std::string headerStrA = header.first + ": " + header.second;
            headerStr = std::wstring(headerStrA.begin(), headerStrA.end());
#endif
            WinHttpAddRequestHeaders(
                hRequest,
                headerStr.c_str(),
                (DWORD)-1,
                WINHTTP_ADDREQ_FLAG_ADD);
        }

        std::wstring contentType = L"Content-Type: application/json";
        WinHttpAddRequestHeaders(
            hRequest,
            contentType.c_str(),
            (DWORD)-1,
            WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);

        std::wstring jsonW(jsonA.begin(), jsonA.end());
        if (!WinHttpSendRequest(
            hRequest,
            WINHTTP_NO_ADDITIONAL_HEADERS,
            0,
            (LPVOID)jsonW.c_str(),
            (DWORD)jsonW.size() * sizeof(wchar_t),
            (DWORD)jsonW.size() * sizeof(wchar_t),
            0))
        {
            throw std::runtime_error("WinHttpSendRequest failed");
        }

        if (!WinHttpReceiveResponse(hRequest, NULL))
            throw std::runtime_error("WinHttpReceiveResponse failed");

        DWORD statusCode = 0;
        DWORD statusSize = sizeof(statusCode);
        WinHttpQueryHeaders(
            hRequest,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX,
            &statusCode,
            &statusSize,
            WINHTTP_NO_HEADER_INDEX);

        success = (statusCode >= 200 && statusCode < 300);

        if (!success)
        {
            helpers::getLogLog().warn(
                LOG4CPLUS_TEXT("WebhookAppender - HTTP status code: ")
                + helpers::convertIntegerToString(static_cast<unsigned long>(statusCode)));
        }
    }
    catch (const std::exception& e)
    {
        helpers::getLogLog().error(
            LOG4CPLUS_TEXT("WebhookAppender::sendWebhook() - Exception: ")
            + LOG4CPLUS_C_STR_TO_TSTRING(e.what()));
    }
    catch (...)
    {
        helpers::getLogLog().error(
            LOG4CPLUS_TEXT("WebhookAppender::sendWebhook() - Unknown exception"));
    }

    if (hRequest)
        WinHttpCloseHandle(hRequest);
    if (hConnect)
        WinHttpCloseHandle(hConnect);
    if (hSession)
        WinHttpCloseHandle(hSession);

    return success;
#else
    helpers::getLogLog().warn(
        LOG4CPLUS_TEXT("WebhookAppender - Not implemented on this platform"));
    return false;
#endif
}


tstring
WebhookAppender::formatJSON(const spi::InternalLoggingEvent& event) const
{
    tostringstream oss;
    
    oss << LOG4CPLUS_TEXT("{\n");
    oss << LOG4CPLUS_TEXT("  \"logger\": \"") << event.getLoggerName() << LOG4CPLUS_TEXT("\",\n");
    oss << LOG4CPLUS_TEXT("  \"level\": \"") << log4cplus::getLogLevelManager().toString(event.getLogLevel()) << LOG4CPLUS_TEXT("\",\n");
    
    tstring message = event.getMessage();
    oss << LOG4CPLUS_TEXT("  \"message\": \"");
    for (tchar c : message)
    {
        if (c == LOG4CPLUS_TEXT('"'))
            oss << LOG4CPLUS_TEXT("\\\"");
        else if (c == LOG4CPLUS_TEXT('\\'))
            oss << LOG4CPLUS_TEXT("\\\\");
        else if (c == LOG4CPLUS_TEXT('\n'))
            oss << LOG4CPLUS_TEXT("\\n");
        else if (c == LOG4CPLUS_TEXT('\r'))
            oss << LOG4CPLUS_TEXT("\\r");
        else if (c == LOG4CPLUS_TEXT('\t'))
            oss << LOG4CPLUS_TEXT("\\t");
        else
            oss << c;
    }
    oss << LOG4CPLUS_TEXT("\",\n");
    
    oss << LOG4CPLUS_TEXT("  \"timestamp\": \"") << event.getTimestamp() << LOG4CPLUS_TEXT("\",\n");
    oss << LOG4CPLUS_TEXT("  \"thread\": \"") << event.getThread() << LOG4CPLUS_TEXT("\",\n");
    oss << LOG4CPLUS_TEXT("  \"file\": \"") << event.getFile() << LOG4CPLUS_TEXT("\",\n");
    oss << LOG4CPLUS_TEXT("  \"line\": ") << event.getLine() << LOG4CPLUS_TEXT(",\n");
    oss << LOG4CPLUS_TEXT("  \"function\": \"") << event.getFunction() << LOG4CPLUS_TEXT("\"\n");
    oss << LOG4CPLUS_TEXT("}");
    
    return oss.str();
}


void
WebhookAppender::parseHeaders(const tstring& headersStr)
{
    if (headersStr.empty())
        return;

    std::vector<tstring> headerList;
    helpers::tokenize(headersStr, LOG4CPLUS_TEXT(','), std::back_inserter(headerList));

    for (const auto& header : headerList)
    {
        size_t pos = header.find(LOG4CPLUS_TEXT(':'));
        if (pos != tstring::npos)
        {
            tstring key = header.substr(0, pos);
            tstring value = header.substr(pos + 1);
            
            key = trim(key);
            value = trim(value);
            
            if (!key.empty())
                headers.push_back(std::make_pair(key, value));
        }
    }
}


void
WebhookAppender::parseKeywords(const tstring& keywordsStr)
{
    if (keywordsStr.empty())
        return;

    std::vector<tstring> keywordList;
    helpers::tokenize(keywordsStr, LOG4CPLUS_TEXT(','), std::back_inserter(keywordList));

    for (const auto& keyword : keywordList)
    {
        tstring trimmed = trim(keyword);
        if (!trimmed.empty())
            keywords.push_back(trimmed);
    }
}

} // namespace log4cplus
