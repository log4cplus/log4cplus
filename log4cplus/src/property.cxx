// Module:  Log4CPLUS
// File:    property.cxx
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

#include <log4cplus/helpers/property.h>
#include <iostream>
#include <fstream>

using namespace std;

#define BUFFER_SIZE 2048

log4cplus::helpers::Properties::Properties() 
{
}



log4cplus::helpers::Properties::Properties(std::istream& input)
{
    init(input);
}




log4cplus::helpers::Properties::Properties(const std::string& inputFile)
{
    ifstream file;
    file.open(inputFile.c_str());
    if(!file) {
        return;
    }
    init(file);
}



void 
log4cplus::helpers::Properties::init(std::istream& input) 
{
    if(input.fail()) {
        return;
    }

    char buffer[BUFFER_SIZE];
    while(!input.eof()) {
        input.getline(buffer, BUFFER_SIZE);
        string tmp(buffer);
        int idx = tmp.find('=');
        if(idx != string::npos) {
            setProperty(tmp.substr(0, idx), tmp.substr(idx + 1));
        }
    }
}




log4cplus::helpers::Properties::~Properties() 
{
}



string
log4cplus::helpers::Properties::getProperty(const string& key) const 
{
    StringMap::const_iterator it = data.find(key);
    if(it == data.end()) {
        return "";
    }
    else {
        return it->second;
    }
}



string
log4cplus::helpers::Properties::getProperty(const string& key,
                                            const string& defaultVal) const 
{
    if(exists(key)) {
        return getProperty(key);
    }
    else {
        return defaultVal;
    }
}


vector<string>
log4cplus::helpers::Properties::propertyNames() const 
{
    vector<string> tmp;
    for(StringMap::const_iterator it=data.begin(); it!=data.end(); ++it) {
        tmp.push_back(it->first);
    }

    return tmp;
}



void
log4cplus::helpers::Properties::setProperty(const std::string& key, 
                                            const std::string& value) 
{
    data[key] = value;
}

