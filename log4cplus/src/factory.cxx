// Module:  Log4CPLUS
// File:    factory.cxx
// Created: 2/2002
// Author:  Tad E. Smith
//
//
// Copyright (C) The Apache Software Foundation. All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

#include <log4cplus/spi/factory.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/threads.h>


using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::spi;

namespace log4cplus {
    class ConsoleAppenderFactory : public Factory {
        void* createObject(const string& configPrefix,
                           const Properties& props)
        {
            return new log4cplus::ConsoleAppender();
        }

        string getTypeName() { return "log4cplus::ConsoleAppender"; }
    };



    class FileAppenderFactory : public Factory {
        void* createObject(const string& configPrefix,
                           const Properties& props)
        {
            string key = configPrefix + "." + "File";
            if(!props.exists(key)) {
                return NULL;
            }
            else {
                return new log4cplus::FileAppender(props.getProperty(key));
            }
        }

        string getTypeName() { return "log4cplus::FileAppender"; }
    };



    class RollingFileAppenderFactory : public Factory {
        void* createObject(const string& configPrefix,
                           const Properties& props)
        {
            string key = configPrefix + "." + "File";
            if(!props.exists(key)) {
                return NULL;
            }
            else {
                return new log4cplus::RollingFileAppender(props.getProperty(key));
            }
        }

        string getTypeName() { return "log4cplus::RollingFileAppender"; }
    };
}



namespace {

    class _static_FactoryRegistry_initializer {
    public:
        _static_FactoryRegistry_initializer() { 
            FactoryRegistry& registry = log4cplus::spi::getFactoryRegistry();
            registry.registerFactory("log4cplus::ConsoleAppender", 
                                     auto_ptr<Factory>(new ConsoleAppenderFactory()));
            registry.registerFactory("log4cplus::FileAppender", 
                                     auto_ptr<Factory>(new FileAppenderFactory()));
            registry.registerFactory("log4cplus::RollingFileAppender", 
                                     auto_ptr<Factory>(new RollingFileAppenderFactory()));

        }
    } initializer;
}


FactoryRegistry&
log4cplus::spi::getFactoryRegistry()
{
    static FactoryRegistry *instance = NULL;

    if(instance == NULL) {
        instance = new FactoryRegistry();
    }

    return *instance;
}




log4cplus::spi::FactoryRegistry::FactoryRegistry()
: mutex(LOG4CPLUS_MUTEX_CREATE)
{
}



log4cplus::spi::FactoryRegistry::~FactoryRegistry() 
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        for(FactoryMap::iterator it=data.begin(); it!=data.end(); ++it) {
            delete (*it).second;
        }
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
    LOG4CPLUS_MUTEX_FREE( mutex );
}




bool 
log4cplus::spi::FactoryRegistry::exists(const std::string& name) const 
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        return data.find(name) != data.end();
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}




bool
log4cplus::spi::FactoryRegistry::registerFactory(const string& name,
                                                 auto_ptr<Factory> factory)
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        FactoryMap::value_type value(name, factory.release());
        pair<FactoryMap::iterator, bool> ret = data.insert(value);

        if(ret.second) {
            return true;
        }
        else {
            delete value.second;
            return false;
        }
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}



Factory* 
log4cplus::spi::FactoryRegistry::getFactory(const string& name) const
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        if(exists(name)) {
            return data.find(name)->second;
        }
        else {
            return NULL;
        }
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}




vector<string> 
log4cplus::spi::FactoryRegistry::getAllNames() const
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        vector<string> tmp;
        for(FactoryMap::const_iterator it=data.begin(); it!=data.end(); ++it) {
            tmp.push_back( (*it).first );
        }

        return tmp;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}

