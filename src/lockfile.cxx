// -*- C++ -*-
//
//  Copyright (C) 2012, Vaclav Zeman. All rights reserved.
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

#include <log4cplus/config/defines.hxx>

#if defined (LOG4CPLUS_HAVE_SYS_TYPES_H)
#include <sys/types.h>
#endif
#if defined (LOG4CPLUS_HAVE_SYS_STAT_H)
#include <sys/stat.h>
#endif
#if defined (LOG4CPLUS_HAVE_SYS_FILE_H)
#include <sys/file.h>
#endif
#if defined (LOG4CPLUS_HAVE_UNISTD_H)
#include <unistd.h>
#endif
#if defined (LOG4CPLUS_HAVE_FCNTL_H)
#include <fcntl.h>
#endif

#include <stdexcept>
#include <cerrno>

#include <log4cplus/helpers/lockfile.h>
#include <log4cplus/helpers/stringhelper.h>

#if defined (_WIN32)
#else
#  if defined (O_EXLOCK)
#    define LOG4CPLUS_USE_O_EXLOCK
#  elif defined (LOG4CPLUS_HAVE_FCNTL) && defined (F_SETLKW)
#    define LOG4CPLUS_USE_SETLKW
#  elif defined (LOG4CPLUS_HAVE_LOCKF)
#    define LOG4CPLUS_USE_LOCKF
#  elif defined (LOG4CPLUS_HAVE_FLOCK)
#    define LOG4CPLUS_USE_FLOCK
#  endif
#  if defined (LOG4CPLUS_USE_O_EXLOCK) || defined (LOG4CPLUS_USE_SETLKW) \
    || defined (LOG4CPLUS_USE_LOCKF) || defined (LOG4CPLUS_USE_FLOCK)
#    define LOG4CPLUS_USE_POSIX_LOCKING
#  endif
#endif

#if ! defined (LOG4CPLUS_USE_POSIX_LOCKING) && ! defined (_WIN32)
#error "no usable file locking"
#endif

namespace log4cplus { namespace helpers {


#if defined (LOG4CPLUS_USE_POSIX_LOCKING)
int const OPEN_FLAGS = O_RDWR | O_CREAT
#if defined (O_CLOEXEC)
    | O_CLOEXEC
#endif
    ;

mode_t const OPEN_MODE = (S_IRWXU ^ S_IXUSR)
    | (S_IRWXG ^ S_IXGRP)
    | (S_IRWXO ^ S_IXOTH);

#endif

struct LockFile::Impl
{
#if defined (LOG4CPLUS_USE_POSIX_LOCKING)
    int fd;

#elif defined (_WIN32)
    HANDLE fh;

#endif
};


//
//
//

LockFile::LockFile (tstring const & lf)
    : lock_file_name (lf)
    , data (new LockFile::Impl)
{
#if defined (LOG4CPLUS_USE_O_EXLOCK)
    data->fd = -1;

#elif defined (LOG4CPLUS_USE_LOCKF) || defined (LOG4CPLUS_USE_FLOCK) \
    || defined (LOG4CPLUS_USE_SETLKW)
    open (OPEN_FLAGS);

#endif
}


LockFile::~LockFile ()
{
#if defined (LOG4CPLUS_USE_POSIX_LOCKING)
    if (data->fd >= 0)
        ::close (data->fd);

#endif

    delete data;
}


void
LockFile::open (int open_flags) const
{
#if defined (LOG4CPLUS_USE_POSIX_LOCKING)
    data->fd = ::open (LOG4CPLUS_TSTRING_TO_STRING (lock_file_name).c_str (),
        open_flags, OPEN_MODE);
    if (data->fd == -1)
    {
        throw std::runtime_error (
            std::string ("could not open or create file ") + lock_file_name);
    }

#elif defined (_WIN32)
#endif
}


void
LockFile::close () const
{
#if defined (LOG4CPLUS_USE_POSIX_LOCKING)
    ::close (data->fd);

#elif defined (_WIN32)
#endif    
}


void
LockFile::lock () const
{
#if defined (LOG4CPLUS_USE_O_EXLOCK)
    open (OPEN_FLAGS | O_EXLOCK);

#elif defined (LOG4CPLUS_USE_SETLKW)
    int ret = 0;

    do
    {
        struct flock fl;
        fl.l_type = F_WRLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 0;
        ret = fcntl (data->fd, F_SETLKW, &fl);
        if (ret == -1 && errno != EINTR)
            throw std::runtime_error (std::string ("fcntl(F_SETLKW) failed: ")
                + convertIntegerToString (errno));
    }
    while (ret == -1);

#elif defined (LOG4CPLUS_USE_LOCKF)
    int ret = 0;

    do
    {
        ret = lockf (data->fd, F_LOCK, 0);
        if (ret == -1 && errno != EINTR)
            throw std::runtime_error (std::string ("lockf() failed: ")
                + convertIntegerToString (errno));
    }
    while (ret == -1);

#elif defined (LOG4CPLUS_USE_FLOCK)
    int ret = 0;

    do
    {
        ret = flock (data->fd, LOCK_EX);
        if (ret == -1 && errno != EINTR)
            throw std::runtime_error (std::string ("fcntl(F_SETLKW) failed: ")
                + convertIntegerToString (errno));
    }
    while (ret == -1);

#elif defined (_WIN32)
#endif
}


void LockFile::unlock () const
{
#if defined (LOG4CPLUS_USE_O_EXLOCK)
    close ();

#elif defined (LOG4CPLUS_USE_SETLKW)
    struct flock fl;
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    int ret = fcntl (data->fd, F_SETLKW, &fl);
    if (ret != 0)
        throw std::runtime_error (std::string ("fcntl(F_SETLKW) failed: ")
            + convertIntegerToString (errno));

#elif defined (LOG4CPLUS_USE_LOCKF)
    int ret = lockf (data->fd, F_ULOCK, 0);
    if (ret != 0)
        throw std::runtime_error (std::string ("lockf() failed: ")
            + convertIntegerToString (errno));

#elif defined (LOG4CPLUS_USE_FLOCK)
    int ret = flock (data->fd, LOCK_UN);
    if (ret != 0)
        throw std::runtime_error (std::string ("flock() failed: ")
            + convertIntegerToString (errno));

#endif

}



} } // namespace log4cplus { namespace helpers {
