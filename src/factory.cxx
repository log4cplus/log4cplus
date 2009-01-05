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

#define APPENDER_FACTORY_DEF(factoryname, appendername)                     \
    class factoryname : public AppenderFactory {                            \
        SharedAppenderPtr createObject(const Properties& props)             \
        {                                                                   \
            return SharedAppenderPtr(new log4cplus::appendername(props));   \
        }                                                                   \
        tstring const & getTypeName() const {                               \
            static tstring const factory_name(                              \
                LOG4CPLUS_TEXT("log4cplus::")                               \
                LOG4CPLUS_TEXT(#appendername));                             \
            return factory_name;                                            \
        }                                                                   \
    }
    

    APPENDER_FACTORY_DEF (ConsoleAppenderFactory, ConsoleAppender);
    APPENDER_FACTORY_DEF (NullAppenderFactory, NullAppender);
    APPENDER_FACTORY_DEF (FileAppenderFactory, FileAppender);
    APPENDER_FACTORY_DEF (RollingFileAppenderFactory, RollingFileAppender);
    APPENDER_FACTORY_DEF (DailyRollingFileAppenderFactory, DailyRollingFileAppender);
    APPENDER_FACTORY_DEF (SocketAppenderFactory, SocketAppender);
#if defined(_WIN32)
    APPENDER_FACTORY_DEF (NTEventLogAppenderFactory, NTEventLogAppender);
    APPENDER_FACTORY_DEF (Win32DebugAppenderFactory, Win32DebugAppender);
#elif defined(LOG4CPLUS_HAVE_SYSLOG_H)
    APPENDER_FACTORY_DEF (SysLogAppenderFactory, SysLogAppender);
#endif

#undef APPENDER_FACTORY_DEF


#define LAYOUT_FACTORY_DEF(factoryname, layoutname)                         \
    class factoryname : public LayoutFactory {                              \
        std::auto_ptr<Layout> createObject(const Properties& props) {       \
             std::auto_ptr<Layout> tmp(new log4cplus::layoutname(props));   \
             return tmp;                                                    \
        }                                                                   \
        tstring const & getTypeName() const {                               \
            static tstring const factory_name(                              \
                LOG4CPLUS_TEXT("log4cplus::")                               \
                LOG4CPLUS_TEXT(#layoutname));                               \
            return factory_name;                                            \
        }                                                                   \
        static tstring const factory_name;                                  \
    }
    

    LAYOUT_FACTORY_DEF (SimpleLayoutFactory, SimpleLayout);
    LAYOUT_FACTORY_DEF (TTCCLayoutFactory, TTCCLayout);
    LAYOUT_FACTORY_DEF (PatternLayoutFactory, PatternLayout);

#undef LAYOUT_FACTORY_DEF


#define LOG4CPLUS_STRINGIFY2(arg) #arg
#define LOG4CPLUS_STRINGIFY(arg) LOG4CPLUS_STRINGIFY2(arg)

#define FILTER_FACTORY_DEF(factoryname, filtername)                         \
    class factoryname : public FilterFactory {                              \
        FilterPtr createObject(const Properties&) {                         \
            return FilterPtr(new log4cplus::spi::filtername);               \
        }                                                                   \
        tstring const & getTypeName() const {                               \
            static tstring const factory_name(                              \
                LOG4CPLUS_TEXT("log4cplus::spi::")                          \
                LOG4CPLUS_TEXT(#filtername));                               \
            return factory_name;                                            \
        }                                                                   \
    }
    

    FILTER_FACTORY_DEF (DenyAllFilterFactory, DenyAllFilter);
    FILTER_FACTORY_DEF (LogLevelMatchFilterFactory, LogLevelMatchFilter);
    FILTER_FACTORY_DEF (LogLevelRangeFilterFactory, LogLevelRangeFilter);
    FILTER_FACTORY_DEF (StringMatchFilterFactory, StringMatchFilter);

#undef FILTER_FACTORY_DEF


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
