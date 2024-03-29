//  Copyright (C) 2009-2017, Vaclav Haisman. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without modifica-
//  tion, are permitted provided that the following conditions are met:
//
//  1. Redistributions of  source code must  retain the above copyright  notice,
//    this list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
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

#ifndef LOG4CPLUS_BOOST_DEVICEAPPENDER_HXX
#define LOG4CPLUS_BOOST_DEVICEAPPENDER_HXX

#include <log4cplus/config.hxx>

#if defined (LOG4CPLUS_HAVE_PRAGMA_ONCE)
#pragma once
#endif

#include <memory>
#include <boost/iostreams/operations.hpp>
#include <boost/shared_ptr.hpp>
#include <log4cplus/appender.h>
#include <log4cplus/helpers/property.h>


namespace log4cplus
{


namespace device_appender_detail
{


template <typename T>
struct device_type_traits
{
    typedef T & device_type;

    static inline
    device_type
    unwrap (device_type x)
    {
        return x;
    }
};


template <typename T>
struct device_type_traits<boost::shared_ptr<T> >
{
    typedef boost::shared_ptr<T> device_type;

    static inline
    T &
    unwrap (device_type const & ptr)
    {
        return *ptr;
    }
};


template <typename T>
struct device_type_traits<std::shared_ptr<T> >
{
    typedef std::shared_ptr<T> device_type;

    static inline
    T &
    unwrap (device_type const & ptr)
    {
        return *ptr;
    }
};


} // namespace device_appender_detail


/**
 * @brief This appender wraps Boost IOStreams' Device concept instance
 * as underlying sink for the appender.
 *
 * @tparam Device Boost IOStreams' Device concept instance
 */
template <typename Device>
class DeviceAppender
    : public Appender
{
public:
    typedef device_appender_detail::device_type_traits<Device> device_traits;
    typedef typename device_traits::device_type device_type;

    template <typename D>
    DeviceAppender (D & d, bool close_device = true)
        : device (d)
        , close_flag (close_device)
    { }

    template <typename D>
    DeviceAppender (boost::shared_ptr<D> const & d, bool close_device = true)
        : device (d)
        , close_flag (close_device)
    { }

    template <typename D>
    DeviceAppender (std::shared_ptr<D> const & d, bool close_device = true)
        : device (d)
        , close_flag (close_device)
    { }

    template <typename D>
    DeviceAppender (D & d, const helpers::Properties & props)
        : Appender (props)
        , device (d)
        , close_flag (true)
    {
        props.getBool (close_flag, LOG4CPLUS_TEXT ("CloseDevice"));
    }

    template <typename D>
    DeviceAppender (boost::shared_ptr<D> const & d,
        const helpers::Properties & props)
        : Appender (props)
        , device (d)
        , close_flag (true)
    {
        props.getBool (close_flag, LOG4CPLUS_TEXT ("CloseDevice"));
    }

    template <typename D>
    DeviceAppender (std::shared_ptr<D> const & d,
        const helpers::Properties & props)
        : Appender (props)
        , device (d)
        , close_flag (true)
    {
        props.getBool (close_flag, LOG4CPLUS_TEXT ("CloseDevice"));
    }

    DeviceAppender (DeviceAppender const &) = delete;
    DeviceAppender & operator = (DeviceAppender const &) = delete;

    virtual
    ~DeviceAppender ()
    {
        destructorImpl ();
    }

    virtual
    void
    close () override
    {
        if (close_flag)
            boost::iostreams::close (device_traits::unwrap (device));
    }

protected:
    virtual
    void
    append (log4cplus::spi::InternalLoggingEvent const & event) override
    {
        tstring & str = formatEvent (event);
        boost::iostreams::write (device_traits::unwrap (device),
            str.c_str (), str.size ());
    }

    device_type device;
    bool close_flag;
};


template <typename T>
inline
SharedAppenderPtr
make_device_appender (T & d, bool close_device = true)
{
    return SharedAppenderPtr (new DeviceAppender<T> (d, close_device));
}


template <typename T>
inline
SharedAppenderPtr
make_device_appender (T & d, const helpers::Properties & props)
{
    return SharedAppenderPtr (new DeviceAppender<T> (d, props));
}


template <typename T>
inline
SharedAppenderPtr
make_device_appender_sp (boost::shared_ptr<T> const & p,
    bool close_device = true)
{
    return SharedAppenderPtr (
        new DeviceAppender<boost::shared_ptr<T> > (p, close_device));
}


template <typename T>
inline
SharedAppenderPtr
make_device_appender_sp (boost::shared_ptr<T> const & p,
    const helpers::Properties & props)
{
    return SharedAppenderPtr (
        new DeviceAppender<boost::shared_ptr<T> > (p, props));
}


template <typename T>
inline
SharedAppenderPtr
make_device_appender_sp (std::shared_ptr<T> const & p,
    bool close_device = true)
{
    return SharedAppenderPtr (
        new DeviceAppender<std::shared_ptr<T> > (p, close_device));
}


template <typename T>
inline
SharedAppenderPtr
make_device_appender_sp (std::shared_ptr<T> const & p,
    const helpers::Properties & props)
{
    return SharedAppenderPtr (
        new DeviceAppender<std::shared_ptr<T> > (p, props));
}


} // namespace log4cplus


#endif // LOG4CPLUS_BOOST_DEVICEAPPENDER_HXX
