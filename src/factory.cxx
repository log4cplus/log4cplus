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
#include <log4cplus/Win32DebugAppender.h>
#endif

namespace log4cplus
{
	using namespace spi;
	using namespace helpers;


	namespace
	{
		
		template <typename T, typename REGISTRY, typename CHAR_TYPE>
		REGISTRY&
		register_factory (REGISTRY & reg, const CHAR_TYPE& name)
		{
			reg.put(std::auto_ptr<typename REGISTRY::product_type>(new T(LOG4CPLUS_TEXT(name))));
			return reg;
		}

		template<typename T>
		struct NameFactory
		{
			static log4cplus::tstring name();
		};

		
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
			const log4cplus::tstring& getTypeName() const 
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

		template<>tstring NameFactory<spi::AppenderFactory>::name() {return LOG4CPLUS_TEXT("log4cplus::");}
		template<>tstring NameFactory<spi::LayoutFactory>::name() {return LOG4CPLUS_TEXT("log4cplus::");}
		template<>tstring NameFactory<spi::FilterFactory>::name() {return LOG4CPLUS_TEXT("log4cplus::spi::");}

		// Special case: DenyAllFilter accepts no properties
		template<>FilterPtr obj_creator<DenyAllFilter, FilterFactory, FilterPtr>::createObject(const helpers::Properties& props) 
		{
			// no-op  (to avoid warning about not using props)
			if (0) props.size();
			return FilterPtr(new DenyAllFilter());

		}

	} // anon namespace


	void initializeFactoryRegistry()
	{
		AppenderFactoryRegistry& reg = getAppenderFactoryRegistry();

		register_factory<obj_creator<ConsoleAppender, AppenderFactory, SharedAppenderPtr> >(reg, "ConsoleAppender");
		register_factory<obj_creator<NullAppender, AppenderFactory, SharedAppenderPtr> >(reg, "NullAppender");
		register_factory<obj_creator<FileAppender, AppenderFactory, SharedAppenderPtr> >(reg, "FileAppender");
		register_factory<obj_creator<RollingFileAppender, AppenderFactory, SharedAppenderPtr> >(reg, "RollingFileAppender");
		register_factory<obj_creator<DailyRollingFileAppender, AppenderFactory, SharedAppenderPtr> >(reg, "DailyRollingFileAppender");
		register_factory<obj_creator<SocketAppender, AppenderFactory, SharedAppenderPtr> >(reg, "SocketAppender");
#if defined(_WIN32)
		register_factory<obj_creator<NTEventLogAppender, AppenderFactory, SharedAppenderPtr> >(reg, "NTEventLogAppender");
		register_factory<obj_creator<Win32DebugAppender, AppenderFactory, SharedAppenderPtr> >(reg, "Win32DebugAppender");
#elif defined(LOG4CPLUS_HAVE_SYSLOG_H)
		register_factory<obj_creator<SysLogAppender, AppenderFactory, SharedAppenderPtr> >(reg, "SysLogAppender");
#endif
#ifndef LOG4CPLUS_SINGLE_THREADED
		register_factory<obj_creator<AsyncAppender, AppenderFactory, SharedAppenderPtr> >(reg, "AsyncAppender");
#endif

		LayoutFactoryRegistry& reg2 = getLayoutFactoryRegistry();
		register_factory<obj_creator<SimpleLayout,  LayoutFactory, std::auto_ptr<Layout> > >(reg2, "SimpleLayout");
		register_factory<obj_creator<TTCCLayout,    LayoutFactory, std::auto_ptr<Layout> > >(reg2, "TTCCLayout");
		register_factory<obj_creator<PatternLayout, LayoutFactory, std::auto_ptr<Layout> > >(reg2, "PatternLayout");

		FilterFactoryRegistry& reg3 = getFilterFactoryRegistry();
		register_factory<obj_creator<DenyAllFilter,       FilterFactory, FilterPtr > >(reg3, "DenyAllFilter");
		register_factory<obj_creator<LogLevelMatchFilter, FilterFactory, FilterPtr > >(reg3, "LogLevelMatchFilter");
		register_factory<obj_creator<LogLevelRangeFilter, FilterFactory, FilterPtr > >(reg3, "LogLevelRangeFilter");
		register_factory<obj_creator<StringMatchFilter,   FilterFactory, FilterPtr > >(reg3, "StringMatchFilter");
	}

	namespace spi 
	{

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

		AppenderFactoryRegistry& getAppenderFactoryRegistry() 
		{
			return meyers_singleton<AppenderFactoryRegistry>();
		}


		LayoutFactoryRegistry& getLayoutFactoryRegistry()
		{
			return meyers_singleton<LayoutFactoryRegistry>();
		}


		FilterFactoryRegistry& getFilterFactoryRegistry()
		{
			return meyers_singleton<FilterFactoryRegistry>();
		}


	} // namespace spi


} // namespace log4cplus

