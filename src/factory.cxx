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

#include <log4cplus/spi/factory.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/fileappender.h>
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

        string getTypeName() { return "log4cplus::ConsoleAppender"; }
    };



    class FileAppenderFactory : public AppenderFactory {
        SharedAppenderPtr createObject(const Properties& props)
        {
            return SharedAppenderPtr(new log4cplus::FileAppender(props));
        }

        string getTypeName() { return "log4cplus::FileAppender"; }
    };



    class RollingFileAppenderFactory : public AppenderFactory {
        SharedAppenderPtr createObject(const Properties& props)
        {
            return SharedAppenderPtr(new log4cplus::RollingFileAppender(props));
        }

        string getTypeName() { return "log4cplus::RollingFileAppender"; }
    };


    class SimpleLayoutFactory : public LayoutFactory {
        std::auto_ptr<Layout> createObject(const Properties& props)
        {
            return std::auto_ptr<Layout>(new log4cplus::SimpleLayout(props));
        }

        string getTypeName() { return "log4cplus::SimpleLayout"; }
    };


    class TTCCLayoutFactory : public LayoutFactory {
        std::auto_ptr<Layout> createObject(const Properties& props)
        {
            return std::auto_ptr<Layout>(new log4cplus::TTCCLayout(props));
        }

        string getTypeName() { return "log4cplus::TTCCLayout"; }
    };


    class PatternLayoutFactory : public LayoutFactory {
        std::auto_ptr<Layout> createObject(const Properties& props)
        {
            return std::auto_ptr<Layout>(new log4cplus::PatternLayout(props));
        }

        string getTypeName() { return "log4cplus::PatternLayout"; }
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
            reg.put("log4cplus::ConsoleAppender", 
                    auto_ptr<AppenderFactory>(new ConsoleAppenderFactory()));
            reg.put("log4cplus::FileAppender", 
                    auto_ptr<AppenderFactory>(new FileAppenderFactory()));
            reg.put("log4cplus::RollingFileAppender", 
                    auto_ptr<AppenderFactory>(new RollingFileAppenderFactory()));

            LayoutFactoryRegistry& reg2 = getLayoutFactoryRegistry();
            reg2.put("log4cplus::SimpleLayout", 
                     auto_ptr<LayoutFactory>(new SimpleLayoutFactory()));
            reg2.put("log4cplus::TTCCLayout", 
                     auto_ptr<LayoutFactory>(new TTCCLayoutFactory()));
            reg2.put("log4cplus::PatternLayout", 
                     auto_ptr<LayoutFactory>(new PatternLayoutFactory()));
        }
    } initializer;
}



///////////////////////////////////////////////////////////////////////////////
// public methods
///////////////////////////////////////////////////////////////////////////////

AppenderFactoryRegistry&
log4cplus::spi::getAppenderFactoryRegistry()
{
    static AppenderFactoryRegistry *instance = NULL;

    if(instance == NULL) {
        instance = new AppenderFactoryRegistry();
    }

    return *instance;
}



LayoutFactoryRegistry&
log4cplus::spi::getLayoutFactoryRegistry()
{
    static LayoutFactoryRegistry *instance = NULL;

    if(instance == NULL) {
        instance = new LayoutFactoryRegistry();
    }

    return *instance;
}



