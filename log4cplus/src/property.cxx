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

#include <cstring>
#if defined (UNICODE)
#  include <cwctype>
#else
#  include <cctype>
#endif
#include <log4cplus/helpers/property.h>
#include <log4cplus/fstreams.h>


namespace log4cplus
{


const tchar helpers::Properties::PROPERTIES_COMMENT_CHAR = LOG4CPLUS_TEXT('#');


namespace
{


static
int
is_space (tchar ch)
{
#if defined (UNICODE)
    return std::iswspace (ch);
#else
    return std::isspace (ch);
#endif
}


static
void
trim_leading_ws (tstring & str)
{
    tstring::iterator it = str.begin ();
    for (; it != str.end (); ++it)
    {
        if (! is_space (*it))
            break;
    }
    str.erase (str.begin (), it);
}


static
void
trim_trailing_ws (tstring & str)
{
    tstring::reverse_iterator rit = str.rbegin ();
    for (; rit != str.rend (); ++rit)
    {
        if (! is_space (*rit))
            break;
    }
    str.erase (rit.base (), str.end ());
}


static
void
trim_ws (tstring & str)
{
    trim_trailing_ws (str);
    trim_leading_ws (str);
}


} // namespace



///////////////////////////////////////////////////////////////////////////////
// helpers::Properties ctors and dtor
///////////////////////////////////////////////////////////////////////////////

helpers::Properties::Properties() 
{
}



helpers::Properties::Properties(tistream& input)
{
    init(input);
}



helpers::Properties::Properties(const tstring& inputFile)
{
    if (inputFile.length() == 0)
        return;

    tifstream file (LOG4CPLUS_TSTRING_TO_STRING(inputFile).c_str());
    init(file);
}



void 
helpers::Properties::init(tistream& input) 
{
    if (! input)
        return;

    tstring buffer;
    while (std::getline (input, buffer))
    {
        trim_leading_ws (buffer);

        if (buffer[0] == PROPERTIES_COMMENT_CHAR)
            continue;
        
        // Check if we have a trailing \r because we are 
        // reading a properties file produced on Windows.
        tstring::size_type const buffLen = buffer.size ();
        if (buffLen > 0 && buffer[buffLen-1] == LOG4CPLUS_TEXT('\r'))
            // Remove trailing 'Windows' \r.
            buffer.resize (buffLen - 1);
        tstring::size_type const idx = buffer.find('=');
        if (idx != tstring::npos)
        {
            tstring key = buffer.substr(0, idx);
            tstring value = buffer.substr(idx + 1);
            trim_trailing_ws (key);
            trim_ws (value);
            setProperty(key, value);
        }
    }
}



helpers::Properties::~Properties() 
{
}



///////////////////////////////////////////////////////////////////////////////
// helpers::Properties public methods
///////////////////////////////////////////////////////////////////////////////

tstring
helpers::Properties::getProperty(const tstring& key) const 
{
    StringMap::const_iterator it (data.find(key));
    if (it == data.end())
        return LOG4CPLUS_TEXT("");
    else
        return it->second;
}



tstring
helpers::Properties::getProperty(const tstring& key,
                                 const tstring& defaultVal) const
{
    StringMap::const_iterator it (data.find (key));
    if (it == data.end ())
        return defaultVal;
    else
        return it->second;
}


std::vector<tstring>
helpers::Properties::propertyNames() const 
{
    std::vector<tstring> tmp;
    for (StringMap::const_iterator it=data.begin(); it!=data.end(); ++it)
        tmp.push_back(it->first);

    return tmp;
}



void
helpers::Properties::setProperty(const tstring& key, const tstring& value)
{
    data[key] = value;
}


bool
helpers::Properties::removeProperty(const tstring& key)
{
    return (data.erase(key) > 0);
}


helpers::Properties 
helpers::Properties::getPropertySubset(
    const tstring& prefix) const
{
    Properties ret;
    std::vector<tstring> const keys = propertyNames();
    size_t const prefix_len = prefix.size ();
    for (std::vector<tstring>::const_iterator it = keys.begin();
        it != keys.end(); ++it)
    {
        int result = it->compare (0, prefix_len, prefix);
        if (result == 0)
            ret.setProperty (it->substr (prefix_len), getProperty(*it));
    }

    return ret;
}


} // namespace log4cplus
