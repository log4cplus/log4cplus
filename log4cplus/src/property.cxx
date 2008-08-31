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
// Revision 1.13  2004/02/10 07:36:37  tcsmith
// Fixed UNICODE support.
//
// Revision 1.12  2003/10/22 05:46:38  tcsmith
// Now strip trailing \r.  (A property file can be created on Windows and used
// on *nix.)
//
// Revision 1.11  2003/06/09 23:06:54  tcsmith
// Added support for "comment" lines in the property file.
//
// Revision 1.10  2003/05/19 15:58:26  tcsmith
// Added a check in the ctor for a valid filename.  If if is not valid, do
// not attempt to open that file.
//
// Revision 1.9  2003/05/01 19:54:30  tcsmith
// Fixed: "warning: comparison between signed and unsigned".
//
// Revision 1.8  2003/05/01 19:35:25  tcsmith
// Fixed VC++ compiler "performance warning".
//
// Revision 1.7  2003/04/19 23:04:32  tcsmith
// Fixed UNICODE support.
//
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

#include <cstring>
#include <log4cplus/helpers/property.h>
#include <log4cplus/fstreams.h>

using namespace std;
using namespace log4cplus;

#define BUFFER_SIZE 2048

const tchar helpers::Properties::PROPERTIES_COMMENT_CHAR = LOG4CPLUS_TEXT('#');



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
    if(inputFile.length() == 0) {
        return;
    }

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
        if(buffer[0] != PROPERTIES_COMMENT_CHAR)
        {
            // Check if we have a trailing \r because we are 
            // reading a properties file produced on Windows.
            size_t buffLen = 
#ifdef UNICODE
				wcslen(buffer);
#else
				strlen(buffer);
#endif
            if((buffLen > 0) && buffer[buffLen-1] == '\r') {
                // Remove trailing 'Windows' \r
                buffer[buffLen-1] = '\0';
            }

            tstring tmp(buffer);
            tstring::size_type idx = tmp.find('=');
            if(idx != tstring::npos) {
                setProperty(tmp.substr(0, idx), tmp.substr(idx + 1));
            }
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
    return (data.erase(key) > 0);
}


log4cplus::helpers::Properties 
log4cplus::helpers::Properties::getPropertySubset(const log4cplus::tstring& prefix) const
{
    Properties ret;

    vector<tstring> keys = propertyNames();
    for(vector<tstring>::iterator it=keys.begin(); it!=keys.end(); ++it) {
        tstring::size_type pos = (*it).find(prefix);
        if(pos != tstring::npos) {
            ret.setProperty( (*it).substr(prefix.size()), getProperty(*it) );
        }
    }

    return ret;
}


