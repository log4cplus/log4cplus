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
// $Log: not supported by cvs2svn $
// Revision 1.17  2004/02/14 20:13:05  tcsmith
// Added support for Win32DebugAppender.
//
// Revision 1.16  2003/09/28 04:02:14  tcsmith
// Made some fixes for the VisualAge 5 compiler on AIX.
//
// Revision 1.15  2003/08/27 14:55:16  tcsmith
// Modified initializeFactoryRegistry() to fix for the AIX compiler.
//
// Revision 1.14  2003/08/08 05:34:04  tcsmith
// Changed the #if checks to look for _WIN32 and not WIN32.
//
// Revision 1.13  2003/07/30 05:51:22  tcsmith
// Modified to remove "unused parameter" warning.
//
// Revision 1.12  2003/06/23 20:15:00  tcsmith
// Added NullAppenderFactory.
//
// Revision 1.11  2003/06/12 23:12:35  tcsmith
// Added DailyRollingFileAppenderFactory implementation.
//
// Revision 1.10  2003/05/28 17:39:12  tcsmith
// Added Filter factories.
//
// Revision 1.9  2003/05/04 07:08:23  tcsmith
// Replaced the static initialization class with the initializeFactoryRegistry()
// method.
//
// Revision 1.8  2003/04/19 20:59:01  tcsmith
// Added NTEventLogAppender support.
//
// Revision 1.7  2003/04/19 07:23:06  tcsmith
// Added SysLogAppenderFactory.
//
// Revision 1.6  2003/04/18 21:00:41  tcsmith
// Converted from std::string to log4cplus::tstring.
//
// Revision 1.5  2003/04/12 13:51:08  tcsmith
// No longer dynamically allocate the object in the "singleton" method.
//
// Revision 1.4  2003/04/03 00:41:11  tcsmith
// Standardized the formatting.
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

using namespace std;
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


#elif defined(HAVE_SYSLOG_H)
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

} // namespace log4cplus



///////////////////////////////////////////////////////////////////////////////
// LOCAL file methods 
///////////////////////////////////////////////////////////////////////////////

namespace log4cplus {
    void initializeFactoryRegistry()
    {
        AppenderFactoryRegistry& reg = getAppenderFactoryRegistry();
        auto_ptr<AppenderFactory> ptr1(new ConsoleAppenderFactory());
        reg.put(ptr1);
        auto_ptr<AppenderFactory> ptr2(new NullAppenderFactory());
        reg.put(ptr2);
        auto_ptr<AppenderFactory> ptr3(new FileAppenderFactory());
        reg.put(ptr3);
        auto_ptr<AppenderFactory> ptr4(new RollingFileAppenderFactory());
        reg.put(ptr4);
        auto_ptr<AppenderFactory> ptr5(new DailyRollingFileAppenderFactory());
        reg.put(ptr5);
        auto_ptr<AppenderFactory> ptr6(new SocketAppenderFactory());
        reg.put(ptr6);
#if defined(_WIN32)
        auto_ptr<AppenderFactory> ptr7(new NTEventLogAppenderFactory());
        reg.put(ptr7);
        auto_ptr<AppenderFactory> ptr8(new Win32DebugAppenderFactory());
        reg.put(ptr8);
#elif defined(HAVE_SYSLOG_H)
        auto_ptr<AppenderFactory> ptr9(new SysLogAppenderFactory());
        reg.put(ptr9);
#endif

        LayoutFactoryRegistry& reg2 = getLayoutFactoryRegistry();
        auto_ptr<LayoutFactory> ptr21(new SimpleLayoutFactory());
        reg2.put(ptr21);
        auto_ptr<LayoutFactory> ptr22(new TTCCLayoutFactory());
        reg2.put(ptr22);
        auto_ptr<LayoutFactory> ptr23(new PatternLayoutFactory());
        reg2.put(ptr23);

        FilterFactoryRegistry& reg3 = getFilterFactoryRegistry();
        auto_ptr<FilterFactory> ptr31(new DenyAllFilterFactory());
        reg3.put(ptr31);
        auto_ptr<FilterFactory> ptr32(new LogLevelMatchFilterFactory());
        reg3.put(ptr32);
        auto_ptr<FilterFactory> ptr33(new LogLevelRangeFilterFactory());
        reg3.put(ptr33);
        auto_ptr<FilterFactory> ptr34(new StringMatchFilterFactory());
        reg3.put(ptr34);
    }
}



///////////////////////////////////////////////////////////////////////////////
// public methods
///////////////////////////////////////////////////////////////////////////////

AppenderFactoryRegistry&
log4cplus::spi::getAppenderFactoryRegistry()
{
    static AppenderFactoryRegistry singleton;
    return singleton;
}



LayoutFactoryRegistry&
log4cplus::spi::getLayoutFactoryRegistry()
{
    static LayoutFactoryRegistry singleton;
    return singleton;
}



FilterFactoryRegistry&
log4cplus::spi::getFilterFactoryRegistry()
{
    static FilterFactoryRegistry singleton;
    return singleton;
}



