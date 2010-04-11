#include <sstream>
#include <log4cplus/helpers/syncprims.h>


namespace log4cplus { namespace thread { namespace detail {


LOG4CPLUS_EXPORT
void
syncprims_throw_exception (char const * const msg, char const * const file,
    int line)
{
    std::ostringstream oss;
    oss << file << ":" << line << msg;
    throw std::runtime_error (oss.str ());
}


} } } // namespace log4cplus { namespace thread { namespace detail {
