// Module:  Log4CPLUS
// File:    property.cxx
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

#include <cstring>
#if defined (UNICODE)
#  include <cwctype>
#else
#  include <cctype>
#endif
#include <fstream>
#include <sstream>
#include <log4cplus/streams.h>
#include <log4cplus/fstreams.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/internal/internal.h>


namespace log4cplus { namespace helpers {


const tchar Properties::PROPERTIES_COMMENT_CHAR = LOG4CPLUS_TEXT('#');


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
    if (inputFile.length() == 0)
        return;

    tifstream file (LOG4CPLUS_TSTRING_TO_STRING(inputFile).c_str());
    init(file);
}



void 
Properties::init(log4cplus::tistream& input) 
{
    if (! input)
        return;

    tstring buffer;
    while (std::getline (input, buffer))
    {
        trim_leading_ws (buffer);

        tstring::size_type const buffLen = buffer.size ();
        if (buffLen == 0 || buffer[0] == PROPERTIES_COMMENT_CHAR)
            continue;
        
        // Check if we have a trailing \r because we are 
        // reading a properties file produced on Windows.
        if (buffer[buffLen-1] == LOG4CPLUS_TEXT('\r'))
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



Properties::~Properties() 
{
}



///////////////////////////////////////////////////////////////////////////////
// helpers::Properties public methods
///////////////////////////////////////////////////////////////////////////////


bool
Properties::exists(const log4cplus::tstring& key) const
{
    return data.find(key) != data.end();
}


bool
Properties::exists(tchar const * key) const
{
    return data.find(key) != data.end();
}


tstring const &
Properties::getProperty(const tstring& key) const 
{
    return get_property_worker (key);
}


log4cplus::tstring const &
Properties::getProperty(tchar const * key) const
{
    return get_property_worker (key);
}


tstring
Properties::getProperty(const tstring& key, const tstring& defaultVal) const
{
    StringMap::const_iterator it (data.find (key));
    if (it == data.end ())
        return defaultVal;
    else
        return it->second;
}


std::vector<tstring>
Properties::propertyNames() const 
{
    std::vector<tstring> tmp;
    for (StringMap::const_iterator it=data.begin(); it!=data.end(); ++it)
        tmp.push_back(it->first);

    return tmp;
}



void
Properties::setProperty(const log4cplus::tstring& key,
    const log4cplus::tstring& value)
{
    data[key] = value;
}


bool
Properties::removeProperty(const log4cplus::tstring& key)
{
    return data.erase(key) > 0;
}


Properties 
Properties::getPropertySubset(const log4cplus::tstring& prefix) const
{
    Properties ret;
    size_t const prefix_len = prefix.size ();
    std::vector<tstring> keys = propertyNames();
    for (std::vector<tstring>::iterator it=keys.begin(); it!=keys.end(); ++it)
    {
        int result = it->compare (0, prefix_len, prefix);
        if (result == 0)
            ret.setProperty (it->substr (prefix_len), getProperty(*it));
    }

    return ret;
}


bool
Properties::getInt (int & val, log4cplus::tstring const & key) const
{
    return get_type_val_worker (val, key);
}


bool
Properties::getUInt (unsigned & val, log4cplus::tstring const & key) const
{
    return get_type_val_worker (val, key);
}


bool
Properties::getLong (long & val, log4cplus::tstring const & key) const
{
    return get_type_val_worker (val, key);
}


bool
Properties::getULong (unsigned long & val, log4cplus::tstring const & key) const
{
    return get_type_val_worker (val, key);
}


bool
Properties::getBool (bool & val, log4cplus::tstring const & key) const
{
    if (! exists (key))
        return false;

    log4cplus::tstring const & prop_val = getProperty (key);
    log4cplus::tistringstream iss (prop_val);

    log4cplus::tstring word;
    if (! (iss >> word))
        return false;
    tchar ch;
    if (iss >> ch)
        return false;
    word = helpers::toLower (word);

    bool result = true;
    if (word == LOG4CPLUS_TEXT ("true"))
        val = true;
    else if (word == LOG4CPLUS_TEXT ("false"))
        val = false;
    else
    {
        iss.clear ();
        iss.seekg (0);
        assert (iss);

        long lval;
        iss >> lval;
        result = !! iss && ! (iss >> ch);
        if (result)
            val = !! lval;
    }

    return result;
}


template <typename StringType>
log4cplus::tstring const &
Properties::get_property_worker (StringType const & key) const
{
    StringMap::const_iterator it (data.find (key));
    if (it == data.end ())
        return log4cplus::internal::empty_str;
    else
        return it->second;
}


template <typename ValType>
bool
Properties::get_type_val_worker (ValType & val, log4cplus::tstring const & key)
    const
{
    if (! exists (key))
        return false;

    log4cplus::tstring const & prop_val = getProperty (key);
    log4cplus::tistringstream iss (prop_val);
    ValType tmp_val;
    tchar ch;

    iss >> tmp_val;
    if (! iss)
        return false;
    iss >> ch;
    if (iss)
        return false;

    val = tmp_val;
    return true;
}


} } // namespace log4cplus { namespace helpers {
