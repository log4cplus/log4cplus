// Module:  Log4CPLUS
// File:    property.cxx
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

#include <log4cplus/fstreams.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/internal/internal.h>

#include <algorithm>
#include <cstring>
#include <functional>
#include <iostream>
#include <ios>
#if defined (UNICODE)
#  include <cwctype>
#else
#  include <cctype>
#endif

namespace log4cplus { namespace helpers {


namespace {

static const tchar PROPERTIES_COMMENT_CHAR = LOG4CPLUS_TEXT('#');

struct IsNotSpace 
{
#if defined (UNICODE)
	bool operator()(int c) const { return (std::iswspace(c) ? false : true); }
#else
	bool operator()(int c) const { return (std::isspace(c) ? false : true); }
#endif
};
	
/**
 * Removes all leading spaces from the input string
 */
tstring&
trim_left (tstring& input)
{
	input.erase(input.begin(), 
				std::find_if(input.begin(), input.end(), IsNotSpace()));
	return input;
}

tstring
trim_left_copy(const tstring& input)
{
	return tstring(std::find_if(input.begin(), input.end(), IsNotSpace()), 
				   input.end());
}


tstring&
trim_right(tstring& input)
{
	input.erase(std::find_if(input.rbegin(), input.rend(), IsNotSpace()).base(),
				input.end());
	return input;
}

tstring
trim_right_copy(const tstring& input)
{
	return tstring(input.begin(), 
				   std::find_if(input.rbegin(), 
								input.rend(), 
								IsNotSpace()).base());

}


//static tstring& trim(tstring & input) {return trim_left(trim_right(input));}

static tstring
trim_copy(const tstring& input)
{
	tstring returnValue(input);
	return trim_left(trim_right(returnValue));
}


} // namespace



///////////////////////////////////////////////////////////////////////////////
// Properties ctors and dtor
///////////////////////////////////////////////////////////////////////////////

Properties::Properties() 
{
}



Properties::Properties(log4cplus::tistream& input)
{
	init(input);
}



Properties::Properties(const log4cplus::tstring& inputFile)
{
	if (!inputFile.empty())
	{
		tifstream tmp(LOG4CPLUS_TSTRING_TO_STRING(inputFile).c_str());
		init(tmp);
	}
}



void 
Properties::init(log4cplus::tistream& input) 
{
	tstring buffer;
	while (!input.fail() && std::getline (input, buffer))
	{
		trim_left (buffer);
		
		if (!buffer.empty() && (*buffer.begin() != PROPERTIES_COMMENT_CHAR))
		{		
			// Check if we have a trailing \r because we are 
			// reading a properties file produced on Windows.
			if (*buffer.rend() == LOG4CPLUS_TEXT('\r')) 
				buffer.resize (buffer.size() - 1);
			
			const tstring::size_type idx = buffer.find('=');
			if (idx != tstring::npos)
				setProperty(trim_right_copy(buffer.substr(0, idx)),
								trim_copy(buffer.substr(idx + 1)));
		}
	}
}



Properties::~Properties() 
{
}



///////////////////////////////////////////////////////////////////////////////
// helpers::Properties public methods
///////////////////////////////////////////////////////////////////////////////

const tstring&
Properties::getProperty(const tstring& key) const 
{
	const map_type::const_iterator it (data_map.find(key));
	return (it == data_map.end() ? log4cplus::internal::empty_str : it->second);
}



const tstring&
Properties::getProperty(const tstring& key, const tstring& defaultVal) const
{
	const map_type::const_iterator it = data_map.find (key);
	return (it == data_map.end() ? defaultVal : it->second);
}


void
Properties::setProperty(const log4cplus::tstring& key,
						const log4cplus::tstring& value)
{
	data_map.insert(std::make_pair(key, value));
	keys.insert(key);			
}


bool
Properties::removeProperty(const log4cplus::tstring& key)
{
	keys.erase(key);
	return data_map.erase(key) > 0;
}


Properties 
Properties::getPropertySubset(const log4cplus::tstring& prefix) const
{
	Properties ret;
	for (map_type::const_iterator iter = data_map.begin(); 
		 iter != data_map.end(); ++iter)
	{
		if (iter->first.find(prefix) == 0)
			ret.setProperty(iter->first.substr(prefix.size()), iter->second);
	}

	return ret;
}


} } // namespace log4cplus { namespace helpers {
