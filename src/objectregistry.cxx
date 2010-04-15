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
#include <log4cplus/helpers/syncprims-pub-impl.h>


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
    thread::Guard guard (mutex);

    return data.find(name) != data.end();
}


std::vector<tstring>
ObjectRegistryBase::getAllNames() const
{
    std::vector<tstring> tmp;

    {
        thread::Guard guard (mutex);
        for(ObjectMap::const_iterator it=data.begin(); it!=data.end(); ++it)
            tmp.push_back( (*it).first );
    }

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

    {
        thread::Guard guard (mutex);
        ret = data.insert(value);
    }

    if (! ret.second)
        deleteObject( value.second );
    return ret.second;
}


void*
ObjectRegistryBase::getVal(const tstring& name) const
{
    thread::Guard guard (mutex);

    ObjectMap::const_iterator it (data.find (name));
    if (it != data.end ())
        return it->second;
    else
        return 0;
}




void
ObjectRegistryBase::clear()
{
    thread::Guard guard (mutex);

    for(ObjectMap::iterator it=data.begin(); it!=data.end(); ++it)
        deleteObject( it->second );
}


} } // namespace log4cplus { namespace spi {
