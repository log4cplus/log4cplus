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
// $Log: not supported by cvs2svn $
// Revision 1.6  2003/04/18 21:52:35  tcsmith
// Converted from using std::ifstream to using log4cplus::tifstream.
//
// Revision 1.5  2003/04/18 21:32:22  tcsmith
// Converted from std::string to log4cplus::tstring.
//
// Revision 1.4  2003/04/05 20:09:17  tcsmith
// Added the removeProperty() method.
//
// Revision 1.3  2003/04/03 01:23:34  tcsmith
// Standardized the formatting.
//

#include <log4cplus/helpers/property.h>
#include <log4cplus/fstreams.h>

using namespace std;
using namespace log4cplus;

#define BUFFER_SIZE 2048



///////////////////////////////////////////////////////////////////////////////
// log4cplus::helpers::Properties ctors and dtor
///////////////////////////////////////////////////////////////////////////////

log4cplus::helpers::Properties::Properties() 
{
}



log4cplus::helpers::Properties::Properties(log4cplus::tistream& input)
{
    init(input);
}



log4cplus::helpers::Properties::Properties(const log4cplus::tstring& inputFile)
{
    tifstream file;
    file.open(LOG4CPLUS_TSTRING_TO_STRING(inputFile).c_str());
    if(!file) {
        return;
    }
    init(file);
}



void 
log4cplus::helpers::Properties::init(log4cplus::tistream& input) 
{
    if(input.fail()) {
        return;
    }

    tchar buffer[BUFFER_SIZE];
    while(!input.eof()) {
        input.getline(buffer, BUFFER_SIZE);
        tstring tmp(buffer);
        int idx = tmp.find('=');
        if(idx != tstring::npos) {
            setProperty(tmp.substr(0, idx), tmp.substr(idx + 1));
        }
    }
}



log4cplus::helpers::Properties::~Properties() 
{
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::helpers::Properties public methods
///////////////////////////////////////////////////////////////////////////////

tstring
log4cplus::helpers::Properties::getProperty(const tstring& key) const 
{
    StringMap::const_iterator it = data.find(key);
    if(it == data.end()) {
        return LOG4CPLUS_TEXT("");
    }
    else {
        return it->second;
    }
}



tstring
log4cplus::helpers::Properties::getProperty(const tstring& key,
                                            const tstring& defaultVal) const 
{
    if(exists(key)) {
        return getProperty(key);
    }
    else {
        return defaultVal;
    }
}


vector<tstring>
log4cplus::helpers::Properties::propertyNames() const 
{
    vector<tstring> tmp;
    for(StringMap::const_iterator it=data.begin(); it!=data.end(); ++it) {
        tmp.push_back(it->first);
    }

    return tmp;
}



void
log4cplus::helpers::Properties::setProperty(const log4cplus::tstring& key, 
                                            const log4cplus::tstring& value) 
{
    data[key] = value;
}


bool
log4cplus::helpers::Properties::removeProperty(const log4cplus::tstring& key)
{
    return data.erase(key);
}


log4cplus::helpers::Properties 
log4cplus::helpers::Properties::getPropertySubset(const log4cplus::tstring& prefix) const
{
    Properties ret;

    vector<tstring> keys = propertyNames();
    for(vector<tstring>::iterator it=keys.begin(); it!=keys.end(); ++it) {
        int pos = (*it).find(prefix);
        if(pos != tstring::npos) {
            ret.setProperty( (*it).substr(prefix.size()), getProperty(*it) );
        }
    }

    return ret;
}


