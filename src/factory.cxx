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
        SharedAppenderPtr createObject(const Properties& props)
        {
            return SharedAppenderPtr(new log4cplus::ConsoleAppender(props));
        }

        tstring getTypeName() { 
	    return LOG4CPLUS_TEXT("log4cplus::ConsoleAppender"); 
	}
    };



    class FileAppenderFactory : public AppenderFactory {
        SharedAppenderPtr createObject(const Properties& props)
        {
            return SharedAppenderPtr(new log4cplus::FileAppender(props));
        }

        tstring getTypeName() { 
	    return LOG4CPLUS_TEXT("log4cplus::FileAppender"); 
	}
    };



    class RollingFileAppenderFactory : public AppenderFactory {
        SharedAppenderPtr createObject(const Properties& props)
        {
            return SharedAppenderPtr(new log4cplus::RollingFileAppender(props));
        }

        tstring getTypeName() { 
	    return LOG4CPLUS_TEXT("log4cplus::RollingFileAppender"); 
	}
    };


#if defined(HAVE_SYSLOG_H)
    class SysLogAppenderFactory : public AppenderFactory {
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
        std::auto_ptr<Layout> createObject(const Properties& props)
        {
            return std::auto_ptr<Layout>(new log4cplus::SimpleLayout(props));
        }

        tstring getTypeName() { 
	    return LOG4CPLUS_TEXT("log4cplus::SimpleLayout"); 
	}
    };


    class TTCCLayoutFactory : public LayoutFactory {
        std::auto_ptr<Layout> createObject(const Properties& props)
        {
            return std::auto_ptr<Layout>(new log4cplus::TTCCLayout(props));
        }

        tstring getTypeName() { 
	    return LOG4CPLUS_TEXT("log4cplus::TTCCLayout"); 
	}
    };


    class PatternLayoutFactory : public LayoutFactory {
        std::auto_ptr<Layout> createObject(const Properties& props)
        {
            return std::auto_ptr<Layout>(new log4cplus::PatternLayout(props));
        }

        tstring getTypeName() { 
	    return LOG4CPLUS_TEXT("log4cplus::PatternLayout"); 
	}
    };
}



///////////////////////////////////////////////////////////////////////////////
// LOCAL file initialization 
///////////////////////////////////////////////////////////////////////////////

namespace {

    class _static_FactoryRegistry_initializer {
    public:
        _static_FactoryRegistry_initializer() { 
            AppenderFactoryRegistry& reg = getAppenderFactoryRegistry();
            reg.put(auto_ptr<AppenderFactory>(new ConsoleAppenderFactory()));
            reg.put(auto_ptr<AppenderFactory>(new FileAppenderFactory()));
            reg.put(auto_ptr<AppenderFactory>(new RollingFileAppenderFactory()));
#if defined(HAVE_SYSLOG_H)
            reg.put(auto_ptr<AppenderFactory>(new SysLogAppenderFactory()));
#endif

            LayoutFactoryRegistry& reg2 = getLayoutFactoryRegistry();
            reg2.put(auto_ptr<LayoutFactory>(new SimpleLayoutFactory()));
            reg2.put(auto_ptr<LayoutFactory>(new TTCCLayoutFactory()));
            reg2.put(auto_ptr<LayoutFactory>(new PatternLayoutFactory()));
        }
    } initializer;
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



