#include <log4cplus/exception.h>

namespace log4cplus {

exception::exception (tstring const & message)
    : std::runtime_error (LOG4CPLUS_TSTRING_TO_STRING (message).c_str ())
{ }

exception::exception (exception const &) = default;

exception & exception::operator=(exception const &) = default;

exception::~exception ()
{ }

} // namespace log4cplus