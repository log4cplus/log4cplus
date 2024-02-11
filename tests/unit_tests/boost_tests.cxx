//  Copyright (C) 2023, Vaclav Haisman. All rights reserved.
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

#include <log4cplus/config.hxx>

#include <log4cplus/loggingmacros.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/boost/deviceappender.hxx>
#include <boost/iostreams/device/back_inserter.hpp>

#if defined (LOG4CPLUS_WITH_UNIT_TESTS)
#include <catch.hpp>


namespace log4cplus
{


namespace io = boost::iostreams;

CATCH_TEST_CASE ("DeviceAppender")
{
    std::unique_ptr<Layout> layout {new SimpleLayout};

    CATCH_SECTION ("direct")
    {
        tstring result;
        auto device (io::back_inserter (result));
        auto appender = make_device_appender (device);
        appender->setLayout (std::move (layout));
        appender->doAppend (spi::InternalLoggingEvent (LOG4CPLUS_TEXT ("logger"),
            INFO_LOG_LEVEL, LOG4CPLUS_TEXT ("message"), __FILE__, __LINE__));
        CATCH_REQUIRE (result.starts_with (LOG4CPLUS_TEXT ("INFO - message")));
    }

    CATCH_SECTION ("boost::shared_ptr")
    {
        tstring result;
        boost::shared_ptr<io::back_insert_device<tstring>> device {
            new io::back_insert_device<tstring> (result)};
        auto appender = make_device_appender (device);
        appender->setLayout (std::move (layout));
        appender->doAppend (spi::InternalLoggingEvent (LOG4CPLUS_TEXT ("logger"),
            INFO_LOG_LEVEL, LOG4CPLUS_TEXT ("message"), __FILE__, __LINE__));
        CATCH_REQUIRE (result.starts_with (LOG4CPLUS_TEXT ("INFO - message")));
    }

    CATCH_SECTION ("std::shared_ptr")
    {
        tstring result;
        std::shared_ptr<io::back_insert_device<tstring>> device {
            new io::back_insert_device<tstring> (result)};
        auto appender = make_device_appender (device);
        appender->setLayout (std::move (layout));
        appender->doAppend (spi::InternalLoggingEvent (LOG4CPLUS_TEXT ("logger"),
            INFO_LOG_LEVEL, LOG4CPLUS_TEXT ("message"), __FILE__, __LINE__));
        CATCH_REQUIRE (result.starts_with (LOG4CPLUS_TEXT ("INFO - message")));
    }
}

} // namespace log4cplus

#endif // defined (LOG4CPLUS_WITH_UNIT_TESTS)