// Module:  Log4CPLUS
// File:    objectregistry.cxx
// Created: 3/2003
// Author:  Tad E. Smith
//
//
// Copyright 2003-2009 Tad E. Smith
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

#include <log4cplus/spi/objectregistry.h>


namespace log4cplus { namespace spi {


///////////////////////////////////////////////////////////////////////////////
// ObjectRegistryBase ctor and dtor
///////////////////////////////////////////////////////////////////////////////

ObjectRegistryBase::ObjectRegistryBase()
 : mutex(LOG4CPLUS_MUTEX_CREATE)
{
}


ObjectRegistryBase::~ObjectRegistryBase()
{
    LOG4CPLUS_MUTEX_FREE( mutex );
}



///////////////////////////////////////////////////////////////////////////////
// ObjectRegistryBase public methods
///////////////////////////////////////////////////////////////////////////////

bool
ObjectRegistryBase::exists(const tstring& name) const
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        return data.find(name) != data.end();
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
}


std::vector<tstring>
ObjectRegistryBase::getAllNames() const
{
    std::vector<tstring> tmp;
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
        for(ObjectMap::const_iterator it=data.begin(); it!=data.end(); ++it)
            tmp.push_back( (*it).first );
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
    return tmp;
}



///////////////////////////////////////////////////////////////////////////////
// ObjectRegistryBase protected methods
///////////////////////////////////////////////////////////////////////////////

bool
ObjectRegistryBase::putVal(const tstring& name, void* object)
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
ObjectRegistryBase::getVal(const tstring& name) const
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
ObjectRegistryBase::clear()
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( mutex )
    for(ObjectMap::iterator it=data.begin(); it!=data.end(); ++it) {
        deleteObject( (*it).second );
    }
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
}


} } // namespace log4cplus { namespace spi {
