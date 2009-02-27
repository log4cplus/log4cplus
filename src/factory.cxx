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

#ifndef LOG4CPLUS_SINGLE_THREADED
#include <log4cplus/asyncappender.h>
#endif

#if defined (_WIN32)
#include <log4cplus/win32debugappender.h>
#endif

using namespace log4cplus;

namespace
{
		
	template <typename T, typename REGISTRY>
	REGISTRY&
	register_factory (REGISTRY & reg, const tstring& name)
	{
		reg.put(std::auto_ptr<typename REGISTRY::product_type>(new T(name)));
		return reg;
	}

	template<typename T>
	struct NameFactory
	{
		static tstring name() {return LOG4CPLUS_TEXT("log4cplus::");}
	};

	// Specialization for FilterFactories. They belong to the spi namespace
	template<>tstring NameFactory<spi::FilterFactory>::name() 
	{
		return LOG4CPLUS_TEXT("log4cplus::spi::");
	}

		
	template<typename T, typename BASE, typename R>
	struct obj_creator : public BASE
	{
		typedef R return_type;

		obj_creator(const tstring& name)
		{
			factory_name = NameFactory<BASE>::name() + name;
		}

		return_type createObject(const helpers::Properties& props) 
		{
			return return_type(new T(props));
		}
		const tstring& getTypeName() const 
		{
			return factory_name;
		}

		tstring factory_name;

	};

	template<typename T> T& meyers_singleton()
	{
		static T data; 
		return data;
	}


	// Special case: DenyAllFilter accepts no properties
	template<>spi::FilterPtr 
	obj_creator<spi::DenyAllFilter, 
				spi::FilterFactory, 
				spi::FilterPtr>::createObject(const helpers::Properties& props) 
	{
		// no-op  (to avoid warning about not using props)
		if (0) props.size();
		return spi::FilterPtr(new spi::DenyAllFilter());

	}

} // anon namespace

spi::BaseFactory::~BaseFactory()
{ }

spi::AppenderFactory::AppenderFactory()
{ }

spi::AppenderFactory::~AppenderFactory()
{ }


spi::LayoutFactory::LayoutFactory()
{ }

spi::LayoutFactory::~LayoutFactory()
{ }

spi::FilterFactory::FilterFactory()
{ }

spi::FilterFactory::~FilterFactory()
{ }

spi::LoggerFactory::~LoggerFactory()
{ }
		
#define LOG4CPLUS_STRINGIFY(arg) LOG4CPLUS_TEXT(#arg)
#define LOG4CPLUS_STRINGIFY2(arg) LOG4CPLUS_STRINGIFY(arg)

#define REGISTER_FACTORY(FAC, OBJ, PTR, REG) \
register_factory<obj_creator<OBJ, FAC, PTR > >(REG,LOG4CPLUS_STRINGIFY2(OBJECT));

void
initializeFactoryRegistry()
{
	spi::AppenderFactoryRegistry& reg = 
		meyers_singleton<spi::AppenderFactoryRegistry>();
	spi::LayoutFactoryRegistry& reg2 = 
		meyers_singleton<spi::LayoutFactoryRegistry>();
	spi::FilterFactoryRegistry& reg3 = 
		meyers_singleton<spi::FilterFactoryRegistry>();

	REGISTER_FACTORY(spi::AppenderFactory, 
					 ConsoleAppender, SharedAppenderPtr, reg);
	REGISTER_FACTORY(spi::AppenderFactory, 
					 FileAppender, SharedAppenderPtr, reg);
	REGISTER_FACTORY(spi::AppenderFactory, 
					 RollingFileAppender, SharedAppenderPtr, reg);
	REGISTER_FACTORY(spi::AppenderFactory, 
					 DailyRollingFileAppender, SharedAppenderPtr, reg);
	REGISTER_FACTORY(spi::AppenderFactory, 
					 SocketAppender, SharedAppenderPtr, reg);
#if defined(_WIN32)
	REGISTER_FACTORY(spi::AppenderFactory, 
					 NTEventLogAppender, SharedAppenderPtr, reg);
	REGISTER_FACTORY(spi::AppenderFactory, 
					 Win32DebugAppender, SharedAppenderPtr, reg);
#elif defined(LOG4CPLUS_HAVE_SYSLOG_H)
	REGISTER_FACTORY(spi::AppenderFactory, 
					 SysLogAppender, SharedAppenderPtr, reg);
#endif
#ifndef LOG4CPLUS_SINGLE_THREADED
	REGISTER_FACTORY(spi::AppenderFactory, 
					 AsyncAppender, SharedAppenderPtr, reg);
#endif

	REGISTER_FACTORY(spi::LayoutFactory, 
					 SimpleLayout, std::auto_ptr<Layout>, reg2);
	REGISTER_FACTORY(spi::LayoutFactory, 
					 TTCCLayout, std::auto_ptr<Layout>, reg2);
	REGISTER_FACTORY(spi::LayoutFactory, 
					 PatternLayout, std::auto_ptr<Layout>, reg2);

	REGISTER_FACTORY(spi::FilterFactory, 
					 spi::DenyAllFilter, spi::FilterPtr, reg3);
	REGISTER_FACTORY(spi::FilterFactory, 
					 spi::LogLevelMatchFilter, spi::FilterPtr, reg3);
	REGISTER_FACTORY(spi::FilterFactory, 
					 spi::LogLevelRangeFilter, spi::FilterPtr, reg3);
	REGISTER_FACTORY(spi::FilterFactory, 
					 spi::StringMatchFilter, spi::FilterPtr, reg3);
}

#undef REGISTER_FACTORY
#undef LOG4CPLUS_STRINGIFY2
#undef LOG4CPLUS_STRINGIFY

