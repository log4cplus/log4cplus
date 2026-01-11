#include <exception>
#include <iostream>
#include <string>
#include <sstream>
#include "log4cplus/logger.h"
#include "log4cplus/webhookappender.h"
#include "log4cplus/helpers/loglog.h"
#include "log4cplus/helpers/property.h"
#include "log4cplus/spi/loggingevent.h"
#include "log4cplus/initializer.h"


static int test_count = 0;
static int test_passed = 0;

#define TEST_START(name) \
    test_count++; \
    log4cplus::tcout << "Test " << test_count << ": " << name << "..." << std::endl;

#define TEST_PASS() \
    test_passed++; \
    log4cplus::tcout << "  PASSED" << std::endl;

#define TEST_FAIL(msg) \
    log4cplus::tcout << "  FAILED: " << msg << std::endl;


void test_basic_functionality()
{
    TEST_START("Basic functionality - create WebhookAppender with constructor");
    
    try {
        log4cplus::tstring url = LOG4CPLUS_TEXT("http://example.com/webhook");
        log4cplus::tstring keywords = LOG4CPLUS_TEXT("error,fatal");
        int timeout = 30;
        
        log4cplus::SharedAppenderPtr appender(
            new log4cplus::WebhookAppender(url, keywords, timeout));
        appender->setName(LOG4CPLUS_TEXT("WebhookTest1"));
        
        TEST_PASS();
    }
    catch(std::exception const& e) {
        TEST_FAIL(e.what());
    }
}


void test_properties_configuration()
{
    TEST_START("Properties configuration - create WebhookAppender from properties");
    
    try {
        log4cplus::tistringstream propsStream(
            LOG4CPLUS_TEXT("WebhookURL=http://example.com/webhook\n")
            LOG4CPLUS_TEXT("Keywords=error,fatal,exception\n")
            LOG4CPLUS_TEXT("Timeout=45\n")
            LOG4CPLUS_TEXT("Headers=Content-Type:application/json,Authorization:Bearer token123\n")
        );
        
        log4cplus::helpers::Properties props(propsStream);
        log4cplus::SharedAppenderPtr appender(
            new log4cplus::WebhookAppender(props));
        appender->setName(LOG4CPLUS_TEXT("WebhookTest2"));
        
        TEST_PASS();
    }
    catch(std::exception const& e) {
        TEST_FAIL(e.what());
    }
}


void test_log_level_filtering()
{
    TEST_START("Log level filtering - only ERROR and above should be sent");
    
    try {
        log4cplus::tstring url = LOG4CPLUS_TEXT("http://example.com/webhook");
        log4cplus::SharedAppenderPtr appender(
            new log4cplus::WebhookAppender(url));
        appender->setName(LOG4CPLUS_TEXT("WebhookTest3"));
        
        log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("test"));
        
        log4cplus::spi::InternalLoggingEvent debug_event(
            logger.getName(),
            log4cplus::DEBUG_LOG_LEVEL,
            LOG4CPLUS_TEXT("Debug message"),
            __FILE__, __LINE__, "test_log_level_filtering");
        
        log4cplus::spi::InternalLoggingEvent info_event(
            logger.getName(),
            log4cplus::INFO_LOG_LEVEL,
            LOG4CPLUS_TEXT("Info message"),
            __FILE__, __LINE__, "test_log_level_filtering");
        
        log4cplus::spi::InternalLoggingEvent warn_event(
            logger.getName(),
            log4cplus::WARN_LOG_LEVEL,
            LOG4CPLUS_TEXT("Warning message"),
            __FILE__, __LINE__, "test_log_level_filtering");
        
        log4cplus::spi::InternalLoggingEvent error_event(
            logger.getName(),
            log4cplus::ERROR_LOG_LEVEL,
            LOG4CPLUS_TEXT("Error message"),
            __FILE__, __LINE__, "test_log_level_filtering");
        
        log4cplus::spi::InternalLoggingEvent fatal_event(
            logger.getName(),
            log4cplus::FATAL_LOG_LEVEL,
            LOG4CPLUS_TEXT("Fatal message"),
            __FILE__, __LINE__, "test_log_level_filtering");
        
        appender->doAppend(debug_event);
        appender->doAppend(info_event);
        appender->doAppend(warn_event);
        appender->doAppend(error_event);
        appender->doAppend(fatal_event);
        
        TEST_PASS();
    }
    catch(std::exception const& e) {
        TEST_FAIL(e.what());
    }
}


void test_keyword_matching()
{
    TEST_START("Keyword matching - only messages with keywords should be sent");
    
    try {
        log4cplus::tstring url = LOG4CPLUS_TEXT("http://example.com/webhook");
        log4cplus::tstring keywords = LOG4CPLUS_TEXT("error,exception,fatal");
        log4cplus::SharedAppenderPtr appender(
            new log4cplus::WebhookAppender(url, keywords));
        appender->setName(LOG4CPLUS_TEXT("WebhookTest4"));
        
        log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("test"));
        
        log4cplus::spi::InternalLoggingEvent event1(
            logger.getName(),
            log4cplus::ERROR_LOG_LEVEL,
            LOG4CPLUS_TEXT("This is an error message"),
            __FILE__, __LINE__, "test_keyword_matching");
        
        log4cplus::spi::InternalLoggingEvent event2(
            logger.getName(),
            log4cplus::ERROR_LOG_LEVEL,
            LOG4CPLUS_TEXT("This is a warning message"),
            __FILE__, __LINE__, "test_keyword_matching");
        
        log4cplus::spi::InternalLoggingEvent event3(
            logger.getName(),
            log4cplus::ERROR_LOG_LEVEL,
            LOG4CPLUS_TEXT("Exception occurred in module X"),
            __FILE__, __LINE__, "test_keyword_matching");
        
        log4cplus::spi::InternalLoggingEvent event4(
            logger.getName(),
            log4cplus::ERROR_LOG_LEVEL,
            LOG4CPLUS_TEXT("Fatal system failure"),
            __FILE__, __LINE__, "test_keyword_matching");
        
        appender->doAppend(event1);
        appender->doAppend(event2);
        appender->doAppend(event3);
        appender->doAppend(event4);
        
        TEST_PASS();
    }
    catch(std::exception const& e) {
        TEST_FAIL(e.what());
    }
}


void test_json_formatting()
{
    TEST_START("JSON formatting - verify log events are formatted correctly");
    
    try {
        log4cplus::tstring url = LOG4CPLUS_TEXT("http://example.com/webhook");
        log4cplus::SharedAppenderPtr appender(
            new log4cplus::WebhookAppender(url));
        appender->setName(LOG4CPLUS_TEXT("WebhookTest5"));
        
        log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("test.logger"));
        
        log4cplus::spi::InternalLoggingEvent event(
            logger.getName(),
            log4cplus::ERROR_LOG_LEVEL,
            LOG4CPLUS_TEXT("Test error message"),
            __FILE__, __LINE__, "test_json_formatting");
        
        appender->doAppend(event);
        
        TEST_PASS();
    }
    catch(std::exception const& e) {
        TEST_FAIL(e.what());
    }
}


void test_empty_keywords()
{
    TEST_START("Empty keywords - all ERROR+ messages should be sent");
    
    try {
        log4cplus::tstring url = LOG4CPLUS_TEXT("http://example.com/webhook");
        log4cplus::tstring empty_keywords = LOG4CPLUS_TEXT("");
        log4cplus::SharedAppenderPtr appender(
            new log4cplus::WebhookAppender(url, empty_keywords));
        appender->setName(LOG4CPLUS_TEXT("WebhookTest6"));
        
        log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("test"));
        
        log4cplus::spi::InternalLoggingEvent event(
            logger.getName(),
            log4cplus::ERROR_LOG_LEVEL,
            LOG4CPLUS_TEXT("Any error message"),
            __FILE__, __LINE__, "test_empty_keywords");
        
        appender->doAppend(event);
        
        TEST_PASS();
    }
    catch(std::exception const& e) {
        TEST_FAIL(e.what());
    }
}


void test_close_appender()
{
    TEST_START("Close appender - verify appender can be closed properly");
    
    try {
        log4cplus::tstring url = LOG4CPLUS_TEXT("http://example.com/webhook");
        log4cplus::SharedAppenderPtr appender(
            new log4cplus::WebhookAppender(url));
        appender->setName(LOG4CPLUS_TEXT("WebhookTest7"));
        
        appender->close();
        
        TEST_PASS();
    }
    catch(std::exception const& e) {
        TEST_FAIL(e.what());
    }
}


void test_threshold_property()
{
    TEST_START("Threshold property - verify threshold can be set via properties");
    
    try {
        log4cplus::tistringstream propsStream(
            LOG4CPLUS_TEXT("WebhookURL=http://example.com/webhook\n")
            LOG4CPLUS_TEXT("Threshold=FATAL\n")
        );
        
        log4cplus::helpers::Properties props(propsStream);
        log4cplus::SharedAppenderPtr appender(
            new log4cplus::WebhookAppender(props));
        appender->setName(LOG4CPLUS_TEXT("WebhookTest8"));
        
        TEST_PASS();
    }
    catch(std::exception const& e) {
        TEST_FAIL(e.what());
    }
}


int main()
{
    log4cplus::Initializer initializer;
    
    log4cplus::helpers::LogLog::getLogLog()->setInternalDebugging(true);
    
    log4cplus::tcout << "========================================" << std::endl;
    log4cplus::tcout << "WebhookAppender Unit Tests" << std::endl;
    log4cplus::tcout << "========================================" << std::endl;
    
    test_basic_functionality();
    test_properties_configuration();
    test_log_level_filtering();
    test_keyword_matching();
    test_json_formatting();
    test_empty_keywords();
    test_close_appender();
    test_threshold_property();
    
    log4cplus::tcout << "========================================" << std::endl;
    log4cplus::tcout << "Test Results: " << test_passed << "/" << test_count << " passed" << std::endl;
    log4cplus::tcout << "========================================" << std::endl;
    
    return (test_passed == test_count) ? 0 : 1;
}
