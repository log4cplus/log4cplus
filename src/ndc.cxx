// Module:  Log4CPLUS
// File:    ndc.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright 2001-2009 Tad E. Smith
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

#include <log4cplus/ndc.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/pointer.h>
#include <log4cplus/internal/internal.h>
#include <exception>


namespace log4cplus
{


///////////////////////////////////////////////////////////////////////////////
// public methods
///////////////////////////////////////////////////////////////////////////////

NDC& 
getNDC()
{
    static NDC singleton;
    return singleton;
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::DiagnosticContext ctors
///////////////////////////////////////////////////////////////////////////////

DiagnosticContext::DiagnosticContext(const log4cplus::tstring& message_,
                                     DiagnosticContext const * parent)
 : message(message_),
   fullMessage( (  (parent == NULL) 
                 ? message 
                 : parent->fullMessage + LOG4CPLUS_TEXT(" ") + message) )
{
}


DiagnosticContext::DiagnosticContext(const log4cplus::tstring& message_)
 : message(message_),
   fullMessage(message)
{
}


DiagnosticContext::DiagnosticContext(tchar const * message_,
                                     DiagnosticContext const * parent)
 : message(message_),
   fullMessage( (  (parent == NULL) 
                 ? message 
                 : parent->fullMessage + LOG4CPLUS_TEXT(" ") + message) )
{
}


DiagnosticContext::DiagnosticContext(tchar const * message_)
 : message(message_),
   fullMessage(message)
{
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::NDC ctor and dtor
///////////////////////////////////////////////////////////////////////////////

NDC::NDC() 
{ }


NDC::~NDC() 
{ }



///////////////////////////////////////////////////////////////////////////////
// log4cplus::NDC public methods
///////////////////////////////////////////////////////////////////////////////

void
NDC::clear()
{
    try {
        DiagnosticContextStack* ptr = getPtr();
        DiagnosticContextStack ().swap (*ptr);
    }
    catch(std::exception& e) {
        getLogLog().error(  LOG4CPLUS_TEXT("NDC::clear()- exception occured: ") 
                          + LOG4CPLUS_C_STR_TO_TSTRING(e.what()));
    }
    catch(...) {
        getLogLog().error(LOG4CPLUS_TEXT("NDC::clear()- exception occured"));
    }
}


DiagnosticContextStack
NDC::cloneStack() const
{
    try {
        DiagnosticContextStack* ptr = getPtr();
        return DiagnosticContextStack(*ptr);
    }
    catch(std::exception& e) {
        getLogLog().error(  LOG4CPLUS_TEXT("NDC::cloneStack()- exception occured: ") 
                          + LOG4CPLUS_C_STR_TO_TSTRING(e.what()));
    }
    catch(...) {
        getLogLog().error(LOG4CPLUS_TEXT("NDC::cloneStack()- exception occured"));
    }

    return DiagnosticContextStack();
}


void 
NDC::inherit(const DiagnosticContextStack& stack)
{
    try {
        DiagnosticContextStack* ptr = getPtr();
        DiagnosticContextStack (stack).swap (*ptr);
    }
    catch(std::exception& e) {
        getLogLog().error(  LOG4CPLUS_TEXT("NDC::inherit()- exception occured: ") 
                          + LOG4CPLUS_C_STR_TO_TSTRING(e.what()));
    }
    catch(...) {
        getLogLog().error(LOG4CPLUS_TEXT("NDC::inherit()- exception occured"));
    }
}


log4cplus::tstring const &
NDC::get() const
{
    try {
        DiagnosticContextStack* ptr = getPtr();
        if(!ptr->empty())
            return ptr->back().fullMessage;
    }
    catch(std::exception& e) {
        getLogLog().error(  LOG4CPLUS_TEXT("NDC::get()- exception occured: ") 
                          + LOG4CPLUS_C_STR_TO_TSTRING(e.what()));
    }
    catch(...) {
        getLogLog().error(LOG4CPLUS_TEXT("NDC::get()- exception occured"));
    }

    return log4cplus::internal::empty_str;
}


size_t 
NDC::getDepth() const
{
    try {
        DiagnosticContextStack* ptr = getPtr();
        return ptr->size();
    }
    catch(std::exception& e) {
        getLogLog().error(  LOG4CPLUS_TEXT("NDC::getDepth()- exception occured: ") 
                          + LOG4CPLUS_C_STR_TO_TSTRING(e.what()));
    }
    catch(...) {
        getLogLog().error(LOG4CPLUS_TEXT("NDC::getDepth()- exception occured"));
    }

    return 0;
}


log4cplus::tstring 
NDC::pop()
{
    try {
        DiagnosticContextStack* ptr = getPtr();
        if(!ptr->empty())
        {
            tstring message;
            message.swap (ptr->back ().message);
            ptr->pop_back();
            return message;
        }
    }
    catch(std::exception& e) {
        getLogLog().error(  LOG4CPLUS_TEXT("NDC::pop()- exception occured: ") 
                          + LOG4CPLUS_C_STR_TO_TSTRING(e.what()));
    }
    catch(...) {
        getLogLog().error(LOG4CPLUS_TEXT("NDC::pop()- exception occured"));
    }

    return LOG4CPLUS_TEXT("");
}


void
NDC::pop_void ()
{
    try
    {
        DiagnosticContextStack* ptr = getPtr ();
        if (! ptr->empty ())
            ptr->pop_back ();
    }
    catch(std::exception& e)
    {
        getLogLog().error(  LOG4CPLUS_TEXT("NDC::pop()- exception occured: ") 
                          + LOG4CPLUS_C_STR_TO_TSTRING(e.what()));
    }
    catch(...)
    {
        getLogLog().error(LOG4CPLUS_TEXT("NDC::pop()- exception occured"));
    }
}


log4cplus::tstring const &
NDC::peek() const
{
    try {
        DiagnosticContextStack* ptr = getPtr();
        if(!ptr->empty())
            return ptr->back().message;
    }
    catch(std::exception& e) {
        getLogLog().error(  LOG4CPLUS_TEXT("NDC::peek()- exception occured: ") 
                          + LOG4CPLUS_C_STR_TO_TSTRING(e.what()));
    }
    catch(...) {
        getLogLog().error(LOG4CPLUS_TEXT("NDC::peek()- exception occured"));
    }

    return log4cplus::internal::empty_str;
}


void 
NDC::push(const log4cplus::tstring& message)
{
    push_worker (message);
}


void 
NDC::push(tchar const * message)
{
    push_worker (message);
}


template <typename StringType>
void
NDC::push_worker (StringType const & message)
{
    try
    {
        DiagnosticContextStack* ptr = getPtr();
        if (ptr->empty())
            ptr->push_back( DiagnosticContext(message, NULL) );
        else
        {
            DiagnosticContext const & dc = ptr->back();
            ptr->push_back( DiagnosticContext(message, &dc) );
        }
    }
    catch(std::exception& e)
    {
        getLogLog().error(  LOG4CPLUS_TEXT("NDC::push()- exception occured: ") 
                          + LOG4CPLUS_C_STR_TO_TSTRING(e.what()));
    }
    catch(...)
    {
        getLogLog().error(LOG4CPLUS_TEXT("NDC::push()- exception occured"));
    }
}


void 
NDC::remove()
{
    try {
        DiagnosticContextStack* ptr = getPtr();
        DiagnosticContextStack ().swap (*ptr);
    }
    catch(std::exception& e) {
        getLogLog().error(  LOG4CPLUS_TEXT("NDC::remove()- exception occured: ") 
                          + LOG4CPLUS_C_STR_TO_TSTRING(e.what()));
    }
    catch(...) {
        getLogLog().error(LOG4CPLUS_TEXT("NDC::remove()- exception occured"));
    }
}


void 
NDC::setMaxDepth(size_t maxDepth)
{
    try {
        DiagnosticContextStack* ptr = getPtr();
        while(maxDepth < ptr->size())
            ptr->pop_back();
    }
    catch(std::exception& e) {
        getLogLog().error(  LOG4CPLUS_TEXT("NDC::setMaxDepth()- exception occured: ") 
                          + LOG4CPLUS_C_STR_TO_TSTRING(e.what()));
    }
    catch(...) {
        getLogLog().error(LOG4CPLUS_TEXT("NDC::setMaxDepth()- exception occured"));
    }
}


DiagnosticContextStack* NDC::getPtr() const
{
    internal::per_thread_data * ptd = internal::get_ptd ();
    return &ptd->ndc_dcs;
}


//
//
//

NDCContextCreator::NDCContextCreator(const log4cplus::tstring& msg) 
{ 
    getNDC().push(msg); 
}


NDCContextCreator::NDCContextCreator(tchar const * msg)
{
    getNDC().push(msg);
}


NDCContextCreator::~NDCContextCreator() 
{ 
    getNDC().pop_void(); 
}


} // namespace log4cplus
