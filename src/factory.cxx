// Module:  Log4CPLUS
// File:    factory.cxx
// Created: 2/2002
// Author:  Tad E. Smith
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

#include <log4cplus/spi/factory.h>
#include <log4cplus/spi/loggerfactory.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/nteventlogappender.h>
#include <log4cplus/nullappender.h>
#include <log4cplus/socketappender.h>
#include <log4cplus/syslogappender.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/threads.h>

#if defined (_WIN32)
#include <log4cplus/Win32DebugAppender.h>
#endif


using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::spi;


///////////////////////////////////////////////////////////////////////////////
// LOCAL file class definitions
///////////////////////////////////////////////////////////////////////////////

namespace log4cplus {
    
namespace spi {

    BaseFactory::~BaseFactory()
    { }


    AppenderFactory::AppenderFactory()
    { }

    AppenderFactory::~AppenderFactory()
    { }


    LayoutFactory::LayoutFactory()
    { }

    LayoutFactory::~LayoutFactory()
    { }

    
    FilterFactory::FilterFactory()
    { }

    FilterFactory::~FilterFactory()
    { }


    LoggerFactory::~LoggerFactory()
    { }


} // namespace spi


namespace {

    class ConsoleAppenderFactory : public AppenderFactory {
    public:
        SharedAppenderPtr createObject(const Properties& props)
        {
            return SharedAppenderPtr(new log4cplus::ConsoleAppender(props));
        }

        tstring getTypeName() { 
            return LOG4CPLUS_TEXT("log4cplus::ConsoleAppender"); 
        }
    };



    class NullAppenderFactory : public AppenderFactory {
    public:
        SharedAppenderPtr createObject(const Properties& props)
        {
            return SharedAppenderPtr(new log4cplus::NullAppender(props));
        }

        tstring getTypeName() { 
            return LOG4CPLUS_TEXT("log4cplus::NullAppender"); 
        }
    };



    class FileAppenderFactory : public AppenderFactory {
    public:
        SharedAppenderPtr createObject(const Properties& props)
        {
            return SharedAppenderPtr(new log4cplus::FileAppender(props));
        }

        tstring getTypeName() { 
            return LOG4CPLUS_TEXT("log4cplus::FileAppender"); 
        }
    };



    class RollingFileAppenderFactory : public AppenderFactory {
    public:
        SharedAppenderPtr createObject(const Properties& props)
        {
            return SharedAppenderPtr(new log4cplus::RollingFileAppender(props));
        }

        tstring getTypeName() { 
            return LOG4CPLUS_TEXT("log4cplus::RollingFileAppender"); 
        }
    };


    class DailyRollingFileAppenderFactory : public AppenderFactory {
    public:
        SharedAppenderPtr createObject(const Properties& props)
        {
            return SharedAppenderPtr(new log4cplus::DailyRollingFileAppender(props));
        }

        tstring getTypeName() { 
            return LOG4CPLUS_TEXT("log4cplus::DailyRollingFileAppender"); 
        }
    };


    class SocketAppenderFactory : public AppenderFactory {
    public:
        SharedAppenderPtr createObject(const Properties& props)
        {
            return SharedAppenderPtr(new log4cplus::SocketAppender(props));
        }

        tstring getTypeName() { 
            return LOG4CPLUS_TEXT("log4cplus::SocketAppender"); 
        }
    };


#if defined(_WIN32)
    class NTEventLogAppenderFactory : public AppenderFactory {
    public:
        SharedAppenderPtr createObject(const Properties& props)
        {
            return SharedAppenderPtr(new log4cplus::NTEventLogAppender(props));
        }

        tstring getTypeName() { 
            return LOG4CPLUS_TEXT("log4cplus::NTEventLogAppender"); 
        }
    };

    class Win32DebugAppenderFactory : public AppenderFactory {
    public:
        SharedAppenderPtr createObject(const Properties& props)
        {
            return SharedAppenderPtr(new log4cplus::Win32DebugAppender(props));
        }

        tstring getTypeName() { 
            return LOG4CPLUS_TEXT("log4cplus::Win32DebugAppender"); 
        }
    };

#elif defined(LOG4CPLUS_HAVE_SYSLOG_H)
    class SysLogAppenderFactory : public AppenderFactory {
    public:
        SharedAppenderPtr createObject(const Properties& props)
        {
            return SharedAppenderPtr(new log4cplus::SysLogAppender(props));
        }

        tstring getTypeName() { 
            return LOG4CPLUS_TEXT("log4cplus::SysLogAppender"); 
        }
    };
#endif

    class SimpleLayoutFactory : public LayoutFactory {
    public:
        std::auto_ptr<Layout> createObject(const Properties& props)
        {
             std::auto_ptr<Layout> tmp(new log4cplus::SimpleLayout(props));
             return tmp;
        }

        tstring getTypeName() { 
            return LOG4CPLUS_TEXT("log4cplus::SimpleLayout"); 
        }
    };


    class TTCCLayoutFactory : public LayoutFactory {
    public:
        std::auto_ptr<Layout> createObject(const Properties& props)
        {
            std::auto_ptr<Layout> tmp(new log4cplus::TTCCLayout(props));
            return tmp;
        }

        tstring getTypeName() { 
            return LOG4CPLUS_TEXT("log4cplus::TTCCLayout"); 
        }
    };


    class PatternLayoutFactory : public LayoutFactory {
    public:
        std::auto_ptr<Layout> createObject(const Properties& props)
        {
            std::auto_ptr<Layout> tmp(new log4cplus::PatternLayout(props));
            return tmp;
        }

        tstring getTypeName() { 
            return LOG4CPLUS_TEXT("log4cplus::PatternLayout"); 
        }
    };



    class DenyAllFilterFactory : public FilterFactory {
    public:
        FilterPtr createObject(const Properties&)
        {
            return FilterPtr(new log4cplus::spi::DenyAllFilter());
        }

        tstring getTypeName() { 
            return LOG4CPLUS_TEXT("log4cplus::spi::DenyAllFilter"); 
        }
    };


    class LogLevelMatchFilterFactory : public FilterFactory {
    public:
        FilterPtr createObject(const Properties& props)
        {
            return FilterPtr(new log4cplus::spi::LogLevelMatchFilter(props));
        }

        tstring getTypeName() { 
            return LOG4CPLUS_TEXT("log4cplus::spi::LogLevelMatchFilter"); 
        }
    };


    class LogLevelRangeFilterFactory : public FilterFactory {
    public:
        FilterPtr createObject(const Properties& props)
        {
            return FilterPtr(new log4cplus::spi::LogLevelRangeFilter(props));
        }

        tstring getTypeName() { 
            return LOG4CPLUS_TEXT("log4cplus::spi::LogLevelRangeFilter"); 
        }
    };


    class StringMatchFilterFactory : public FilterFactory {
    public:
        FilterPtr createObject(const Properties& props)
        {
            return FilterPtr(new log4cplus::spi::StringMatchFilter(props));
        }

        tstring getTypeName() { 
            return LOG4CPLUS_TEXT("log4cplus::spi::StringMatchFilter"); 
        }
    };

} // namespace


///////////////////////////////////////////////////////////////////////////////
// LOCAL file methods 
/////////////////////////////////////////////////////////////////////////////// 

namespace
{

template <typename Fac, typename Reg>
static void
reg_factory (Reg & reg)
{
    reg.put (std::auto_ptr<typename Reg::product_type> (new Fac));
}

} // namespace


void initializeFactoryRegistry()
{
    AppenderFactoryRegistry& reg = getAppenderFactoryRegistry();
    reg_factory<ConsoleAppenderFactory> (reg);
    reg_factory<NullAppenderFactory> (reg);
    reg_factory<FileAppenderFactory> (reg);
    reg_factory<RollingFileAppenderFactory> (reg);
    reg_factory<DailyRollingFileAppenderFactory> (reg);
    reg_factory<SocketAppenderFactory> (reg);
#if defined(_WIN32)
    reg_factory<NTEventLogAppenderFactory> (reg);
    reg_factory<Win32DebugAppenderFactory> (reg);
#elif defined(LOG4CPLUS_HAVE_SYSLOG_H)
    reg_factory<SysLogAppenderFactory> (reg);
#endif

    LayoutFactoryRegistry& reg2 = getLayoutFactoryRegistry();
    reg_factory<SimpleLayoutFactory> (reg2);
    reg_factory<TTCCLayoutFactory> (reg2);
    reg_factory<PatternLayoutFactory> (reg2);

    FilterFactoryRegistry& reg3 = getFilterFactoryRegistry();
    reg_factory<DenyAllFilterFactory> (reg3);
    reg_factory<LogLevelMatchFilterFactory> (reg3);
    reg_factory<LogLevelRangeFilterFactory> (reg3);
    reg_factory<StringMatchFilterFactory> (reg3);
}




///////////////////////////////////////////////////////////////////////////////
// public methods
///////////////////////////////////////////////////////////////////////////////

namespace spi
{


AppenderFactoryRegistry&
getAppenderFactoryRegistry()
{
    static AppenderFactoryRegistry singleton;
    return singleton;
}


LayoutFactoryRegistry&
getLayoutFactoryRegistry()
{
    static LayoutFactoryRegistry singleton;
    return singleton;
}


FilterFactoryRegistry&
getFilterFactoryRegistry()
{
    static FilterFactoryRegistry singleton;
    return singleton;
}


} // namespace spi


} // namespace log4cplus
