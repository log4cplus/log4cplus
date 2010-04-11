// Module:  Log4CPLUS
// File:    factory.cxx
// Created: 2/2002
// Author:  Tad E. Smith
//
//
// Copyright 2002-2009 Tad E. Smith
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

#include <log4cplus/spi/factory.h>
#include <log4cplus/spi/loggerfactory.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/nullappender.h>
#include <log4cplus/socketappender.h>
#include <log4cplus/syslogappender.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/threads.h>

#ifndef LOG4CPLUS_SINGLE_THREADED
#include <log4cplus/asyncappender.h>
#endif

#if defined (_WIN32)
#  if defined (LOG4CPLUS_HAVE_NT_EVENT_LOG)
#    include <log4cplus/nteventlogappender.h>
#  endif
#  include <log4cplus/Win32DebugAppender.h>
#endif


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
    class factoryname : public spi::AppenderFactory {                       \
        SharedAppenderPtr createObject(const helpers::Properties& props)    \
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
#ifndef LOG4CPLUS_SINGLE_THREADED
    APPENDER_FACTORY_DEF (AsyncAppenderFactory, AsyncAppender);
#endif
#if defined(_WIN32)
#  if defined (LOG4CPLUS_HAVE_NT_EVENT_LOG)
    APPENDER_FACTORY_DEF (NTEventLogAppenderFactory, NTEventLogAppender);
#  endif
    APPENDER_FACTORY_DEF (Win32DebugAppenderFactory, Win32DebugAppender);
#elif defined(LOG4CPLUS_HAVE_SYSLOG_H)
    APPENDER_FACTORY_DEF (SysLogAppenderFactory, SysLogAppender);
#endif

#undef APPENDER_FACTORY_DEF


#define LAYOUT_FACTORY_DEF(factoryname, layoutname)                         \
    class factoryname : public spi::LayoutFactory {                         \
        std::auto_ptr<Layout>                                               \
        createObject(const helpers::Properties& props) {                    \
             std::auto_ptr<Layout> tmp(new log4cplus::layoutname(props));   \
             return tmp;                                                    \
        }                                                                   \
        tstring const & getTypeName() const {                               \
            static tstring const factory_name(                              \
                LOG4CPLUS_TEXT("log4cplus::")                               \
                LOG4CPLUS_TEXT(#layoutname));                               \
            return factory_name;                                            \
        }                                                                   \
    }
    

    LAYOUT_FACTORY_DEF (SimpleLayoutFactory, SimpleLayout);
    LAYOUT_FACTORY_DEF (TTCCLayoutFactory, TTCCLayout);
    LAYOUT_FACTORY_DEF (PatternLayoutFactory, PatternLayout);

#undef LAYOUT_FACTORY_DEF


#define FILTER_FACTORY_DEF(factoryname, filtername)                         \
    class factoryname : public spi::FilterFactory {                         \
        spi::FilterPtr createObject(const helpers::Properties& props) {     \
            return spi::FilterPtr(new log4cplus::spi::filtername(props));   \
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
    std::auto_ptr<typename Reg::product_type> pfac (new Fac);
    // Force initialization of inner static variable.
    pfac->getTypeName ();
    reg.put (pfac);
}

} // namespace


void initializeFactoryRegistry()
{
    spi::AppenderFactoryRegistry& reg = spi::getAppenderFactoryRegistry();
    reg_factory<ConsoleAppenderFactory> (reg);
    reg_factory<NullAppenderFactory> (reg);
    reg_factory<FileAppenderFactory> (reg);
    reg_factory<RollingFileAppenderFactory> (reg);
    reg_factory<DailyRollingFileAppenderFactory> (reg);
    reg_factory<SocketAppenderFactory> (reg);
#if defined(_WIN32)
#  if defined(LOG4CPLUS_HAVE_NT_EVENT_LOG)
    reg_factory<NTEventLogAppenderFactory> (reg);
#  endif
    reg_factory<Win32DebugAppenderFactory> (reg);
#elif defined(LOG4CPLUS_HAVE_SYSLOG_H)
    reg_factory<SysLogAppenderFactory> (reg);
#endif
#ifndef LOG4CPLUS_SINGLE_THREADED
    reg_factory<AsyncAppenderFactory> (reg);
#endif

    spi::LayoutFactoryRegistry& reg2 = spi::getLayoutFactoryRegistry();
    reg_factory<SimpleLayoutFactory> (reg2);
    reg_factory<TTCCLayoutFactory> (reg2);
    reg_factory<PatternLayoutFactory> (reg2);

    spi::FilterFactoryRegistry& reg3 = spi::getFilterFactoryRegistry();
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
