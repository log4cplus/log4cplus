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

#include <utility>
#include <memory>
#include <type_traits>
#include <log4cplus/mdc.h>
#include <log4cplus/internal/internal.h>

#if defined (LOG4CPLUS_WITH_UNIT_TESTS)
#include <catch.hpp>
#endif


namespace log4cplus
{

//
// MappedDiagnosticContext
//

MappedDiagnosticContext::MappedDiagnosticContext ()
{ }


MappedDiagnosticContext::~MappedDiagnosticContext ()
{ }


void
MappedDiagnosticContext::clear ()
{
    stacks_map.clear ();
    context_map.clear ();
}


//
// MDC
//

MDC::MDC () = default;


MDC::~MDC () = default;


MappedDiagnosticContext *
MDC::getPtr ()
{
    return &internal::get_ptd ()->mdc;
}


void
MDC::clear()
{
    MappedDiagnosticContext * const dc = getPtr ();
    dc->clear ();
}


template <typename Map, typename Key, typename Value>
static
auto
insert_or_assign (Map & map, Key && key, Value && value)
-> std::optional<typename Map::mapped_type>
{
    using ValueType = typename Map::mapped_type;
    auto result = map.emplace (std::forward<Key>(key),
        std::forward<Value> (value));
    if (result.second)
        return std::optional<ValueType> ();
    else
    {
        auto & map_value = result.first->second;
        ValueType old_value {std::move (map_value)};
        map_value = std::forward<Value> (value);
        return std::optional<ValueType> (std::move (old_value));
    }
}


void
MDC::put (tstring_view const & key, tstring const & value)
{
    MappedDiagnosticContextMap & mdc_map = getPtr ()->context_map;
    insert_or_assign (mdc_map, key, value);
}


void
MDC::put (tstring_view const & key, tstring && value)
{
    MappedDiagnosticContextMap & mdc_map = getPtr ()->context_map;
    insert_or_assign (mdc_map, key, std::move (value));
}


template <typename StackMap, typename Key, typename Value>
static
void
push_to_stack (StackMap & stacks_map, Key && key, Value && value)
{
    auto it = stacks_map.find (key);
    if (it != stacks_map.end ())
    {
        MappedDiagnosticContextStack & stack = it->second;
        stack.emplace_back (std::forward<Value> (value));
    }
    else
    {
        stacks_map.emplace (std::piecewise_construct,
            std::forward_as_tuple (std::forward<Key>(key)),
            std::forward_as_tuple (
                MappedDiagnosticContextStack::size_type (1),
                tstring (std::forward<Value> (value))));
    }
}


void
MDC::push (tstring_view const & key, tstring && value)
{
    MappedDiagnosticContext & mdc = *getPtr ();

    MappedDiagnosticContextMap & mdc_map = mdc.context_map;
    auto opt_prev {insert_or_assign (mdc_map, key, std::move (value))};

    if (opt_prev.has_value ())
    {
        MappedDiagnosticContextStacksMap & stacks_map = mdc.stacks_map;
        push_to_stack (stacks_map, key, std::move (*opt_prev));
    }
}


void
MDC::push (tstring_view const & key, tstring_view const & value)
{
    MappedDiagnosticContext & mdc = *getPtr ();

    MappedDiagnosticContextMap & mdc_map = mdc.context_map;
    auto opt_prev {insert_or_assign (mdc_map, key, value)};

    if (opt_prev.has_value ())
    {
        MappedDiagnosticContextStacksMap & stacks_map = mdc.stacks_map;
        push_to_stack (stacks_map, key, std::move (*opt_prev));
    }
}


void
MDC::pop (tstring const & key)
{
    MappedDiagnosticContext & mdc = *getPtr ();

    MappedDiagnosticContextMap & mdc_map = mdc.context_map;
    MappedDiagnosticContextStacksMap & stacks_map = mdc.stacks_map;
    auto it = stacks_map.find (key);
    if (it != stacks_map.end ())
    {
        MappedDiagnosticContextStack & stack = it->second;
        if (! stack.empty ())
        {
            insert_or_assign (mdc_map, key, std::move (stack.back ()));
            stack.pop_back ();
            return;
        }
    }

    mdc_map.erase (key);
}


std::optional<tstring>
MDC::get (tstring_view const & key) const
{
    MappedDiagnosticContextMap const & dc = getPtr ()->context_map;
    auto it = dc.find (key);
    if (it != dc.end ())
        return std::optional<tstring> (it->second);
    else
        return std::optional<tstring> ();
}


void
MDC::remove (tstring const & key)
{
    MappedDiagnosticContextMap & dc = getPtr ()->context_map;
    dc.erase (key);
}


MappedDiagnosticContextMap const &
MDC::getContext () const
{
    return getPtr ()->context_map;
}


//
// MDCGuard
//

MDCGuard::MDCGuard (tstring && key_, tstring && value)
    : key (std::move (key_))
{
    MDC & mdc = getMDC ();
    mdc.push (key, std::move (value));
}


MDCGuard::MDCGuard (tstring const & key_, tstring const & value)
    : key (key_)
{
    MDC & mdc = getMDC ();
    mdc.push (key, value);
}


MDCGuard::~MDCGuard ()
{
    MDC & mdc = getMDC ();
    mdc.pop (key);
}


//
// Tests
//

#if defined (LOG4CPLUS_WITH_UNIT_TESTS)
CATCH_TEST_CASE ("MDC", "[MDC]")
{
    tstring const a {LOG4CPLUS_TEXT ("a")};
    tstring const value1 {LOG4CPLUS_TEXT ("value1")};
    tstring value2 {LOG4CPLUS_TEXT ("value2")};

    tstring str;
    std::optional<tstring> opt_str;
    MDC & mdc = getMDC ();
    mdc.clear ();
    mdc.put (LOG4CPLUS_TEXT ("key1"), LOG4CPLUS_TEXT ("value1"));
    mdc.put (LOG4CPLUS_TEXT ("key2"), LOG4CPLUS_TEXT ("value2"));

    auto & context_map = internal::get_ptd ()->mdc.context_map;
    auto & stacks_map = internal::get_ptd ()->mdc.stacks_map;

    CATCH_SECTION ("hash test")
    {
        using Map = std::unordered_map<tstring, int, helpers::tstring_hash>;
        Map m;
        insert_or_assign (m, LOG4CPLUS_TEXT ("a"), 1);
        CATCH_REQUIRE (m[LOG4CPLUS_TEXT ("a")] == 1);
        insert_or_assign (m, LOG4CPLUS_TEXT ("b"), 2);
        CATCH_REQUIRE (m[LOG4CPLUS_TEXT ("b")] == 2);
    }

    CATCH_SECTION ("map test")
    {
        using Map = std::map<tstring, int>;
        Map m;
        insert_or_assign (m, tstring_view (LOG4CPLUS_TEXT ("a")), 1);
        CATCH_REQUIRE (m[LOG4CPLUS_TEXT ("a")] == 1);
        insert_or_assign (m, tstring_view (LOG4CPLUS_TEXT ("b")), 2);
        CATCH_REQUIRE (m[LOG4CPLUS_TEXT ("b")] == 2);
    }

    CATCH_SECTION ("get")
    {
        CATCH_REQUIRE ((opt_str = mdc.get (LOG4CPLUS_TEXT ("key1"))).has_value ());
        CATCH_REQUIRE (*opt_str == LOG4CPLUS_TEXT ("value1"));
        CATCH_REQUIRE ((opt_str = mdc.get (LOG4CPLUS_TEXT ("key2"))).has_value ());
        CATCH_REQUIRE (*opt_str == LOG4CPLUS_TEXT ("value2"));
        CATCH_REQUIRE (! mdc.get (LOG4CPLUS_TEXT ("nonexisting")).has_value ());
    }

    CATCH_SECTION ("remove")
    {
        mdc.remove (LOG4CPLUS_TEXT ("key1"));
        CATCH_REQUIRE (! mdc.get (LOG4CPLUS_TEXT ("key1")).has_value ());
        CATCH_REQUIRE ((opt_str = mdc.get (LOG4CPLUS_TEXT ("key2"))).has_value ());
        CATCH_REQUIRE (*opt_str == LOG4CPLUS_TEXT ("value2"));
    }

    CATCH_SECTION ("clear")
    {
        mdc.clear ();
        CATCH_REQUIRE (! mdc.get (LOG4CPLUS_TEXT ("key1")).has_value ());
        CATCH_REQUIRE (! mdc.get (LOG4CPLUS_TEXT ("key2")).has_value ());
    }

    CATCH_SECTION ("insert_or_assign")
    {
        MappedDiagnosticContextMap map;

        // Insert a value using lvalues.
        std::optional<tstring> opt1 {insert_or_assign (map, a, value1)};
        CATCH_REQUIRE (value1 == LOG4CPLUS_TEXT ("value1"));
        CATCH_REQUIRE (!opt1.has_value ());
        CATCH_REQUIRE (map[LOG4CPLUS_TEXT ("a")] == LOG4CPLUS_TEXT ("value1"));

        // Insert a different value with the same key.
        std::optional<tstring> opt2 {insert_or_assign (map, a, value2)};
        CATCH_REQUIRE (value2 == LOG4CPLUS_TEXT ("value2"));
        CATCH_REQUIRE (opt2.has_value ());
        CATCH_REQUIRE (*opt2 == LOG4CPLUS_TEXT ("value1"));
        CATCH_REQUIRE (map[LOG4CPLUS_TEXT ("a")] == LOG4CPLUS_TEXT ("value2"));
    }

    CATCH_SECTION ("push_to_stack")
    {
        MappedDiagnosticContextStacksMap map;

        push_to_stack(map, a, value1);
        CATCH_REQUIRE (value1 == LOG4CPLUS_TEXT ("value1"));
        auto & stack = map[a];
        CATCH_REQUIRE (stack.size () == 1);
        CATCH_REQUIRE (std::find (stack.begin (), stack.end (), value1) != stack.end ());

        push_to_stack(map, a, value2);
        CATCH_REQUIRE (value2 == LOG4CPLUS_TEXT ("value2"));
        CATCH_REQUIRE (stack.size () == 2);
        CATCH_REQUIRE (std::find (stack.begin (), stack.end (), value1) != stack.end ());
        CATCH_REQUIRE (std::find (stack.begin (), stack.end (), value2) != stack.end ());
    }

    CATCH_SECTION ("MDCGuard")
    {
        tcerr << LOG4CPLUS_TEXT ("MDCGuard\n");
        {
            mdc.clear ();
            CATCH_REQUIRE (stacks_map.empty ());
            CATCH_REQUIRE (context_map.empty ());

            CATCH_REQUIRE (! mdc.get (LOG4CPLUS_TEXT ("a")).has_value ());
            {
                MDCGuard guard1 (LOG4CPLUS_TEXT ("a"), LOG4CPLUS_TEXT ("value1"));
                CATCH_REQUIRE ((opt_str = mdc.get (LOG4CPLUS_TEXT ("a"))).has_value ());
                CATCH_REQUIRE (*opt_str == LOG4CPLUS_TEXT ("value1"));

                CATCH_REQUIRE (stacks_map.size () == 0);
                CATCH_REQUIRE (context_map.size () == 1);

                {
                    CATCH_REQUIRE (value2 != LOG4CPLUS_TEXT ("value1"));
                    MDCGuard guard2 (LOG4CPLUS_TEXT ("a"), value2);

                    CATCH_REQUIRE (stacks_map.size () == 1);
                    CATCH_REQUIRE (stacks_map[LOG4CPLUS_TEXT ("a")].size () == 1);
                    CATCH_REQUIRE (stacks_map[LOG4CPLUS_TEXT ("a")][0] == value1);
                    CATCH_REQUIRE (context_map.size () == 1);

                    CATCH_REQUIRE ((opt_str = mdc.get (LOG4CPLUS_TEXT ("a"))).has_value ());
                    CATCH_REQUIRE (*opt_str == value2);
                }

                CATCH_REQUIRE (stacks_map.size () == 1);
                CATCH_REQUIRE (stacks_map[LOG4CPLUS_TEXT ("a")].empty ());
                CATCH_REQUIRE (context_map.size () == 1);

                CATCH_REQUIRE ((opt_str = mdc.get (LOG4CPLUS_TEXT ("a"))).has_value ());
                CATCH_REQUIRE (*opt_str == LOG4CPLUS_TEXT ("value1"));
            }
            CATCH_REQUIRE (! mdc.get (LOG4CPLUS_TEXT ("a")).has_value ());
        }
    }
}

#endif

} // namespace log4cplus
