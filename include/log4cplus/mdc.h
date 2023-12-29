// -*- C++ -*-
//  Copyright (C) 2010-2017, Vaclav Haisman. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without modifica-
//  tion, are permitted provided that the following conditions are met:
//
//  1. Redistributions of  source code must  retain the above copyright  notice,
//     this list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED WARRANTIES,
//  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS  FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN NO  EVENT SHALL  THE
//  APACHE SOFTWARE  FOUNDATION  OR ITS CONTRIBUTORS  BE LIABLE FOR  ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLU-
//  DING, BUT NOT LIMITED TO, PROCUREMENT  OF SUBSTITUTE GOODS OR SERVICES; LOSS
//  OF USE, DATA, OR  PROFITS; OR BUSINESS  INTERRUPTION)  HOWEVER CAUSED AND ON
//  ANY  THEORY OF LIABILITY,  WHETHER  IN CONTRACT,  STRICT LIABILITY,  OR TORT
//  (INCLUDING  NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT OF THE  USE OF
//  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef LOG4CPLUS_MDC_H_HEADER
#define LOG4CPLUS_MDC_H_HEADER

#include <log4cplus/config.hxx>

#if defined (LOG4CPLUS_HAVE_PRAGMA_ONCE)
#pragma once
#endif

#include <log4cplus/tstring.h>

#include <map>
#include <unordered_map>
#include <functional>
#include <optional>
#include <deque>


namespace log4cplus
{

//! Mapped diagnostic context stack of previous values.
using MappedDiagnosticContextStack = std::deque<tstring>;

//! Mapped diagnostic context map of keys to stacks of values.
using MappedDiagnosticContextStacksMap
    = std::unordered_map<tstring, MappedDiagnosticContextStack,
        helpers::tstring_hash, std::equal_to<>>;

//! Mapped diagnostic context map, keys to values.
using MappedDiagnosticContextMap = std::map<tstring, tstring,
    std::less<>>;

//! Internal MDC storage.
struct LOG4CPLUS_EXPORT MappedDiagnosticContext final
{
    MappedDiagnosticContext ();
    ~MappedDiagnosticContext ();

    MappedDiagnosticContext(MappedDiagnosticContext const &) = delete;
    MappedDiagnosticContext(MappedDiagnosticContext &&) = delete;

    MappedDiagnosticContext & operator = (MappedDiagnosticContext const &) = delete;
    MappedDiagnosticContext & operator = (MappedDiagnosticContext &&) = delete;

    void clear ();


    //! Backing map for the mapped diagnostic context.
    //! This allows MDCGuard to push and pop values.
    MappedDiagnosticContextStacksMap stacks_map;

    //! Current mapped diagnostic context map.
    //! This map is used for rendering in layouts.
    //! \see MDC::getContext
    MappedDiagnosticContextMap context_map;
};


//! Mapped diagnostic context
class LOG4CPLUS_EXPORT MDC
{
public:
    /**
     * Clear any nested diagnostic information if any. This method is
     * useful in cases where the same thread can be potentially used
     * over and over in different unrelated contexts.
     */
    void clear();

    /**
     * @{
     * \brief Put key-value pair into MDC.
     *
     * \param key MDC key
     * \param value MDC value
     */
    void put (tstring_view const & key, tstring const & value);
    void put (tstring_view const & key, tstring && value);
    /**@}*/

    /**
     * @{
     * \brief Push key-value pair into MDC. It is possible to pop
     * the previous value later.
     *
     * \param key MDC key
     * \param value MDC value
     * \return std::optional<tstring> Return previous value, if any,
     * stored in MDC.
     */
    void push (tstring_view const & key, tstring_view const & value);
    void push (tstring_view const & key, tstring && value);
    /**@}*/

    void pop (tstring const & key);

    std::optional<tstring> get (tstring_view const & key) const;
    void remove (tstring const & key);

    MappedDiagnosticContextMap const & getContext () const;

    // Public ctor and dtor but only to be used by internal::DefaultContext.
    MDC ();
    virtual ~MDC ();

private:
    LOG4CPLUS_PRIVATE static MappedDiagnosticContext * getPtr ();
};


class LOG4CPLUS_EXPORT MDCGuard
{
public:
    MDCGuard (tstring &&, tstring &&);
    MDCGuard (tstring const &, tstring const &);
    ~MDCGuard ();

    MDCGuard (MDCGuard &&) = delete;
    MDCGuard (MDCGuard const &) = delete;
    MDCGuard & operator = (MDCGuard const &) = delete;
    MDCGuard & operator = (MDCGuard &&) = delete;

private:
    tstring key;
};


LOG4CPLUS_EXPORT MDC & getMDC ();


} // namespace log4cplus


#endif // LOG4CPLUS_MDC_H_HEADER
