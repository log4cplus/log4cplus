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
#include <log4cplus/consoleappender.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/nteventlogappender.h>
#include <log4cplus/nullappender.h>
#include <log4cplus/socketappender.h>
#include <log4cplus/syslogappender.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/threads.h>


using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::spi;


///////////////////////////////////////////////////////////////////////////////
// LOCAL file class definitions
///////////////////////////////////////////////////////////////////////////////

namespace log4cplus {
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


#if defined(WIN32)
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
            return std::auto_ptr<Layout>(new log4cplus::SimpleLayout(props));
        }

        tstring getTypeName() { 
            return LOG4CPLUS_TEXT("log4cplus::SimpleLayout"); 
        }
    };


    class TTCCLayoutFactory : public LayoutFactory {
    public:
        std::auto_ptr<Layout> createObject(const Properties& props)
        {
            return std::auto_ptr<Layout>(new log4cplus::TTCCLayout(props));
        }

        tstring getTypeName() { 
            return LOG4CPLUS_TEXT("log4cplus::TTCCLayout"); 
        }
    };


    class PatternLayoutFactory : public LayoutFactory {
    public:
        std::auto_ptr<Layout> createObject(const Properties& props)
        {
            return std::auto_ptr<Layout>(new log4cplus::PatternLayout(props));
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
}



///////////////////////////////////////////////////////////////////////////////
// LOCAL file methods 
///////////////////////////////////////////////////////////////////////////////

namespace log4cplus {
    void initializeFactoryRegistry() {
        AppenderFactoryRegistry& reg = getAppenderFactoryRegistry();
        reg.put(auto_ptr<AppenderFactory>(new ConsoleAppenderFactory()));
        reg.put(auto_ptr<AppenderFactory>(new NullAppenderFactory()));
        reg.put(auto_ptr<AppenderFactory>(new FileAppenderFactory()));
        reg.put(auto_ptr<AppenderFactory>(new RollingFileAppenderFactory()));
        reg.put(auto_ptr<AppenderFactory>(new DailyRollingFileAppenderFactory()));
        reg.put(auto_ptr<AppenderFactory>(new SocketAppenderFactory()));
#if defined(WIN32)
        reg.put(auto_ptr<AppenderFactory>(new NTEventLogAppenderFactory()));
#elif defined(HAVE_SYSLOG_H)
        reg.put(auto_ptr<AppenderFactory>(new SysLogAppenderFactory()));
#endif

        LayoutFactoryRegistry& reg2 = getLayoutFactoryRegistry();
        reg2.put(auto_ptr<LayoutFactory>(new SimpleLayoutFactory()));
        reg2.put(auto_ptr<LayoutFactory>(new TTCCLayoutFactory()));
        reg2.put(auto_ptr<LayoutFactory>(new PatternLayoutFactory()));

        FilterFactoryRegistry& reg3 = getFilterFactoryRegistry();
        reg3.put(auto_ptr<FilterFactory>(new DenyAllFilterFactory()));
        reg3.put(auto_ptr<FilterFactory>(new LogLevelMatchFilterFactory()));
        reg3.put(auto_ptr<FilterFactory>(new LogLevelRangeFilterFactory()));
        reg3.put(auto_ptr<FilterFactory>(new StringMatchFilterFactory()));
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



