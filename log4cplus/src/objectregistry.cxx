// Module:  Log4CPLUS
// File:    objectregistry.cxx
// Created: 3/2003
// Author:  Tad E. Smith
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

#include <log4cplus/spi/objectregistry.h>

using namespace std;
using namespace log4cplus::spi;


///////////////////////////////////////////////////////////////////////////////
// log4cplus::spi::ObjectRegistryBase ctor and dtor
///////////////////////////////////////////////////////////////////////////////

log4cplus::spi::ObjectRegistryBase::ObjectRegistryBase()
 : mutex(LOG4CPLUS_MUTEX_CREATE)
{
}


log4cplus::spi::ObjectRegistryBase::~ObjectRegistryBase()
{
    LOG4CPLUS_MUTEX_FREE( mutex );
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::spi::ObjectRegistryBase public methods
///////////////////////////////////////////////////////////////////////////////

bool
log4cplus::spi::ObjectRegistryBase::exists(const log4cplus::tstring& name) const
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        return data.find(name) != data.end();
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
}


std::vector<log4cplus::tstring>
log4cplus::spi::ObjectRegistryBase::getAllNames() const
{
    std::vector<log4cplus::tstring> tmp;
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        for(ObjectMap::const_iterator it=data.begin(); it!=data.end(); ++it)
            tmp.push_back( (*it).first );
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
    return tmp;
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::spi::ObjectRegistryBase protected methods
///////////////////////////////////////////////////////////////////////////////

bool
log4cplus::spi::ObjectRegistryBase::putVal(const log4cplus::tstring& name, void* object)
{
    ObjectMap::value_type value(name, object);
    std::pair<ObjectMap::iterator, bool> ret;

    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        ret = data.insert(value);
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;

    if (! ret.second)
        deleteObject( value.second );
    return ret.second;
}


void*
log4cplus::spi::ObjectRegistryBase::getVal(const log4cplus::tstring& name) const
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        ObjectMap::const_iterator it (data.find (name));
        if (it != data.end ())
            return it->second;
        else
            return 0;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
}




void
log4cplus::spi::ObjectRegistryBase::clear()
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
    for(ObjectMap::iterator it=data.begin(); it!=data.end(); ++it) {
        deleteObject( (*it).second );
    }
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
}
