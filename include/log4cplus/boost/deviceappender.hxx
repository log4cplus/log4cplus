#ifndef LOG4CPLUS_BOOST_DEVICEAPPENDER_HXX
#define LOG4CPLUS_BOOST_DEVICEAPPENDER_HXX

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/not.hpp>
#include <boost/iostreams/operations.hpp>
#include <boost/shared_ptr.hpp>
#include <log4cplus/appender.h>


namespace log4cplus
{


namespace device_appender_detail
{


template <typename T>
struct device_type_traits
{
    typedef T & device_type;

    static
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

    static
    T &
    unwrap (device_type const & ptr)
    {
        return *ptr;
    }
};


} // namespace device_appender_detail


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
    DeviceAppender (D & d, const helpers::Properties & props)
        : Appender (props)
        , device (d)
    {
        if (props.exists (LOG4CPLUS_TEXT ("CloseDevice")))
            close_flag = true;
        else
            close_flag = false;
    }

    template <typename D>
    DeviceAppender (boost::shared_ptr<D> const & d,
        const helpers::Properties & props)
        : Appender (props)
        , device (d)
    {
        if (props.exists (LOG4CPLUS_TEXT ("CloseDevice")))
            close_flag = true;
        else
            close_flag = false;
    }

    virtual
    ~DeviceAppender ()
    { }

    virtual
    void
    close ()
    {
        if (close_flag)
            boost::iostreams::close (device_traits::unwrap (device));
    }

protected:
    virtual void
    append (log4cplus::spi::InternalLoggingEvent const & event)
    {
        tstring & str = formatEvent (event);
        boost::iostreams::write (device_traits::unwrap (device),
            str.c_str (), str.size ());
    }

    device_type device;
    bool close_flag;

private:
    DeviceAppender (DeviceAppender const &);
    DeviceAppender & operator = (DeviceAppender const &);
};


template <typename T>
inline
SharedAppenderPtr
make_device_appender (T & d, bool close_device = true)
{
    SharedAppenderPtr app (new DeviceAppender<T> (d, close_device));
    return app;
}


template <typename T>
inline
SharedAppenderPtr
make_device_appender (T & d, const helpers::Properties & props)
{
    SharedAppenderPtr app (new DeviceAppender<T> (d, props));
    return app;
}


template <typename T>
inline
SharedAppenderPtr
make_device_appender_sp (boost::shared_ptr<T> const & p,
    bool close_device = true)
{
    SharedAppenderPtr app (
        new DeviceAppender<boost::shared_ptr<T> > (p, close_device));
    return app;
}


template <typename T>
inline
SharedAppenderPtr
make_device_appender_sp (boost::shared_ptr<T> const & p,
    const helpers::Properties & props)
{
    SharedAppenderPtr app (
        new DeviceAppender<boost::shared_ptr<T> > (p, props));
    return app;
}


} // namespace log4cplus


#endif // LOG4CPLUS_BOOST_DEVICEAPPENDER_HXX

