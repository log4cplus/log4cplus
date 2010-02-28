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

#if defined (_WIN32)
#  if defined (LOG4CPLUS_HAVE_NT_EVENT_LOG)
#    include <log4cplus/nteventlogappender.h>
#  endif
#  if defined (LOG4CPLUS_HAVE_WIN32_CONSOLE)
#    include <log4cplus/win32consoleappender.h>
#  endif
#  include <log4cplus/Win32DebugAppender.h>
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


namespace
{


template <typename ProductFactoryBase>
class LocalFactoryBase
    : public ProductFactoryBase
{
public:
    LocalFactoryBase (tchar const * n)
        : name (n)
    { }

    virtual log4cplus::tstring getTypeName()
    {
        return name;
    }

private:
    log4cplus::tstring name;
};


template <typename LocalProduct, typename ProductFactoryBase>
class FactoryTempl
    : public LocalFactoryBase<ProductFactoryBase>
{
public:
    typedef typename ProductFactoryBase::ProductPtr ProductPtr;

    FactoryTempl (tchar const * n)
        : LocalFactoryBase<ProductFactoryBase> (n)
    { }

    virtual ProductPtr createObject (Properties const & props)
    {
        return ProductPtr (new LocalProduct (props));
    }
};


#define REG_APPENDER(reg, appendername)                             \
reg.put (                                                           \
    std::auto_ptr<AppenderFactory> (                                \
        new FactoryTempl<appendername, AppenderFactory> (           \
            LOG4CPLUS_TEXT("log4cplus::")                           \
            LOG4CPLUS_TEXT(#appendername))))


#define REG_LAYOUT(reg, layoutname)                                 \
reg.put (                                                           \
    std::auto_ptr<LayoutFactory> (                                  \
        new FactoryTempl<layoutname, LayoutFactory> (               \
            LOG4CPLUS_TEXT("log4cplus::")                           \
            LOG4CPLUS_TEXT(#layoutname))))


#define REG_FILTER(reg, filtername)                                 \
reg.put (                                                           \
    std::auto_ptr<FilterFactory> (                                  \
        new FactoryTempl<spi::filtername, FilterFactory> (          \
            LOG4CPLUS_TEXT("log4cplus::spi::")                      \
            LOG4CPLUS_TEXT(#filtername))))


} // namespace


///////////////////////////////////////////////////////////////////////////////
// LOCAL file methods 
/////////////////////////////////////////////////////////////////////////////// 


void initializeFactoryRegistry()
{
    AppenderFactoryRegistry& reg = getAppenderFactoryRegistry();
    REG_APPENDER (reg, ConsoleAppender);
    REG_APPENDER (reg, NullAppender);
    REG_APPENDER (reg, FileAppender);
    REG_APPENDER (reg, RollingFileAppender);
    REG_APPENDER (reg, DailyRollingFileAppender);
    REG_APPENDER (reg, SocketAppender);
#if defined(_WIN32)
#  if defined(LOG4CPLUS_HAVE_NT_EVENT_LOG)
    REG_APPENDER (reg, NTEventLogAppender);
#  endif
#  if defined(LOG4CPLUS_HAVE_WIN32_CONSOLE)
    REG_APPENDER (reg, Win32ConsoleAppender);
#  endif
    REG_APPENDER (reg, Win32DebugAppender);
#elif defined(LOG4CPLUS_HAVE_SYSLOG_H)
    REG_APPENDER (reg, SysLogAppender);
#endif

    LayoutFactoryRegistry& reg2 = getLayoutFactoryRegistry();
    REG_LAYOUT (reg2, SimpleLayout);
    REG_LAYOUT (reg2, TTCCLayout);
    REG_LAYOUT (reg2, PatternLayout);

    FilterFactoryRegistry& reg3 = getFilterFactoryRegistry();
    REG_FILTER (reg3, DenyAllFilter);
    REG_FILTER (reg3, LogLevelMatchFilter);
    REG_FILTER (reg3, LogLevelRangeFilter);
    REG_FILTER (reg3, StringMatchFilter);
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
