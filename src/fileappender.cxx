// Module:  Log4CPLUS
// File:    fileappender.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright 2001-2017 Tad E. Smith
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

#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/streams.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/helpers/timehelper.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/helpers/fileinfo.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/thread/syncprims-pub-impl.h>
#include <log4cplus/internal/internal.h>
#include <log4cplus/internal/env.h>
#include <algorithm>
#include <sstream>
#include <cstdio>
#include <stdexcept>
#include <cmath> // std::fmod

// For _wrename() and _wremove() on Windows.
#include <stdio.h>
#include <cerrno>
#ifdef LOG4CPLUS_HAVE_ERRNO_H
#include <errno.h>
#endif

#if defined (LOG4CPLUS_WITH_UNIT_TESTS)
#include <catch.hpp>
#include <fstream>
#include <vector>
#if defined (_WIN32)
#include <direct.h>
#else
#include <sys/stat.h>
#endif
#endif


namespace log4cplus
{

using helpers::Properties;
using helpers::Time;


const long DEFAULT_ROLLING_LOG_SIZE = 10 * 1024 * 1024L;
const long MINIMUM_ROLLING_LOG_SIZE = 200*1024L;


///////////////////////////////////////////////////////////////////////////////
// File LOCAL definitions
///////////////////////////////////////////////////////////////////////////////

namespace
{

long const LOG4CPLUS_FILE_NOT_FOUND = ENOENT;

#if defined (LOG4CPLUS_WITH_UNIT_TESTS)
thread_local Time const * timeBasedAppenderTestTime = nullptr;
#endif

Time
timeBasedAppenderNow ()
{
#if defined (LOG4CPLUS_WITH_UNIT_TESTS)
    if (timeBasedAppenderTestTime)
        return *timeBasedAppenderTestTime;
#endif
    return helpers::now ();
}


static
long
file_rename (tstring const & src, tstring const & target)
{
#if defined (UNICODE) && defined (_WIN32)
    if (_wrename (src.c_str (), target.c_str ()) == 0)
        return 0;
    else
        return errno;

#else
    if (std::rename (LOG4CPLUS_TSTRING_TO_STRING (src).c_str (),
            LOG4CPLUS_TSTRING_TO_STRING (target).c_str ()) == 0)
        return 0;
    else
        return errno;

#endif
}


static
long
file_remove (tstring const & src)
{
#if defined (UNICODE) && defined (_WIN32)
    if (_wremove (src.c_str ()) == 0)
        return 0;
    else
        return errno;

#else
    if (std::remove (LOG4CPLUS_TSTRING_TO_STRING (src).c_str ()) == 0)
        return 0;
    else
        return errno;

#endif
}


static
void
loglog_renaming_result (helpers::LogLog & loglog, tstring const & src,
    tstring const & target, long ret)
{
    if (ret == 0)
    {
        loglog.debug (
            LOG4CPLUS_TEXT("Renamed file ")
            + src
            + LOG4CPLUS_TEXT(" to ")
            + target);
    }
    else if (ret != LOG4CPLUS_FILE_NOT_FOUND)
    {
        tostringstream oss;
        oss << LOG4CPLUS_TEXT("Failed to rename file from ")
            << src
            << LOG4CPLUS_TEXT(" to ")
            << target
            << LOG4CPLUS_TEXT("; error ")
            << ret;
        loglog.error (oss.str ());
    }
}


static
void
loglog_opening_result (helpers::LogLog & loglog,
    log4cplus::tostream const & os, tstring const & filename)
{
    if (! os)
    {
        loglog.error (
            LOG4CPLUS_TEXT("Failed to open file ")
            + filename);
    }
}


static
void
rolloverFiles(const tstring& filename, unsigned int maxBackupIndex)
{
    helpers::LogLog * loglog = helpers::LogLog::getLogLog();

    // Delete the oldest file
    tostringstream buffer;
    buffer << filename << LOG4CPLUS_TEXT(".") << maxBackupIndex;
    long ret = file_remove (buffer.str ());

    tostringstream source_oss;
    tostringstream target_oss;

    // Map {(maxBackupIndex - 1), ..., 2, 1} to {maxBackupIndex, ..., 3, 2}
    for (int i = maxBackupIndex - 1; i >= 1; --i)
    {
        source_oss.str(internal::empty_str);
        target_oss.str(internal::empty_str);

        source_oss << filename << LOG4CPLUS_TEXT(".") << i;
        target_oss << filename << LOG4CPLUS_TEXT(".") << (i+1);

        tstring const source (source_oss.str ());
        tstring const target (target_oss.str ());

#if defined (_WIN32)
        // Try to remove the target first. It seems it is not
        // possible to rename over existing file.
        ret = file_remove (target);
#endif

        ret = file_rename (source, target);
        loglog_renaming_result (*loglog, source, target, ret);
    }
} // end rolloverFiles()

} // namespace


///////////////////////////////////////////////////////////////////////////////
// FileAppenderBase ctors and dtor
///////////////////////////////////////////////////////////////////////////////

FileAppenderBase::FileAppenderBase(const tstring& filename_,
    std::ios_base::openmode mode_, bool immediateFlush_, bool createDirs_)
    : immediateFlush(immediateFlush_)
    , createDirs (createDirs_)
    , reopenDelay(1)
    , bufferSize (0)
    , buffer (nullptr)
    , filename(filename_)
    , localeName (LOG4CPLUS_TEXT ("DEFAULT"))
    , fileOpenMode(mode_)
{ }


FileAppenderBase::FileAppenderBase(const Properties& props,
                                   std::ios_base::openmode mode_)
    : Appender(props)
    , immediateFlush(true)
    , createDirs (false)
    , reopenDelay(1)
    , bufferSize (0)
    , buffer (nullptr)
{
    filename = props.getProperty(LOG4CPLUS_TEXT("File"));
    lockFileName = props.getProperty (LOG4CPLUS_TEXT ("LockFile"));
    localeName = props.getProperty (LOG4CPLUS_TEXT ("Locale"), LOG4CPLUS_TEXT ("DEFAULT"));

    props.getBool (immediateFlush, LOG4CPLUS_TEXT("ImmediateFlush"));
    props.getBool (createDirs, LOG4CPLUS_TEXT("CreateDirs"));
    props.getInt (reopenDelay, LOG4CPLUS_TEXT("ReopenDelay"));
    props.getULong (bufferSize, LOG4CPLUS_TEXT("BufferSize"));

    bool app = (mode_ & (std::ios_base::app | std::ios_base::ate)) != 0;
    props.getBool (app, LOG4CPLUS_TEXT("Append"));
    fileOpenMode = app ? std::ios::app : std::ios::trunc;

    if (props.getProperty(LOG4CPLUS_TEXT("TextMode"), LOG4CPLUS_TEXT("Text"))
        == LOG4CPLUS_TEXT("Binary"))
        fileOpenMode |= std::ios_base::binary;
}


void
FileAppenderBase::init()
{
    if (useLockFile && lockFileName.empty ())
    {
        if (filename.empty())
        {
            getErrorHandler()->error(LOG4CPLUS_TEXT
                ("UseLockFile is true but neither LockFile nor File are specified"));
            return;
        }

        lockFileName = filename;
        lockFileName += LOG4CPLUS_TEXT(".lock");
    }

    if (bufferSize != 0)
    {
        buffer.reset (new tchar[bufferSize]);
        out.rdbuf ()->pubsetbuf (buffer.get (), bufferSize);
    }

    helpers::LockFileGuard guard;
    if (useLockFile && ! lockFile.get ())
    {
        if (createDirs)
            internal::make_dirs (lockFileName);

        try
        {
            lockFile.reset (new helpers::LockFile (lockFileName));
            guard.attach_and_lock (*lockFile);
        }
        catch (std::runtime_error const &)
        {
            // We do not need to do any logging here as the internals
            // of LockFile already use LogLog to report the failure.
            return;
        }
    }

    open(fileOpenMode);
    imbue (internal::get_locale_by_name (localeName));
}

///////////////////////////////////////////////////////////////////////////////
// FileAppenderBase public methods
///////////////////////////////////////////////////////////////////////////////

void
FileAppenderBase::close()
{
    thread::MutexGuard guard (access_mutex);

    out.close();
    buffer.reset ();
    closed = true;
}


std::locale
FileAppenderBase::imbue(std::locale const& loc)
{
    return out.imbue (loc);
}


std::locale
FileAppenderBase::getloc () const
{
    return out.getloc ();
}


///////////////////////////////////////////////////////////////////////////////
// FileAppenderBase protected methods
///////////////////////////////////////////////////////////////////////////////

// This method does not need to be locked since it is called by
// doAppend() which performs the locking
void
FileAppenderBase::append(const spi::InternalLoggingEvent& event)
{
    if(!out.good()) {
        if(!reopen()) {
            getErrorHandler()->error (filename.empty ()
                ? LOG4CPLUS_TEXT ("file is not open")
                : LOG4CPLUS_TEXT ("file is not open: ") + filename);
            return;
        }
        // Resets the error handler to make it
        // ready to handle a future append error.
        else
            getErrorHandler()->reset();
    }

    if (useLockFile)
        out.seekp (0, std::ios_base::end);

    layout->formatAndAppend(out, event);

    if(immediateFlush || useLockFile)
        out.flush();
}

void
FileAppenderBase::open(std::ios_base::openmode mode)
{
    if (createDirs)
        internal::make_dirs (filename);

    out.open(LOG4CPLUS_FSTREAM_PREFERED_FILE_NAME(filename).c_str(), mode);

    if(!out.good()) {
        getErrorHandler()->error(LOG4CPLUS_TEXT("Unable to open file: ") + filename);
        return;
    }
    helpers::getLogLog().debug(LOG4CPLUS_TEXT("Just opened file: ") + filename);
}

bool
FileAppenderBase::reopen()
{
    // When append never failed and the file re-open attempt must
    // be delayed, set the time when reopen should take place.
    if (reopen_time == log4cplus::helpers::Time () && reopenDelay != 0)
        reopen_time = log4cplus::helpers::now ()
            + helpers::chrono::seconds (reopenDelay);
    else
    {
        // Otherwise, check for end of the delay (or absence of delay)
        // to re-open the file.
        if (reopen_time <= log4cplus::helpers::now ()
            || reopenDelay == 0)
        {
            // Close the current file
            out.close();
            // reset flags since the C++ standard specified that all
            // the flags should remain unchanged on a close
            out.clear();

            // Re-open the file.
            open(std::ios_base::out | std::ios_base::ate | std::ios_base::app);

            // Reset last fail time.
            reopen_time = log4cplus::helpers::Time ();

            // Succeed if no errors are found.
            if(out.good())
                return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// FileAppender ctors and dtor
///////////////////////////////////////////////////////////////////////////////

FileAppender::FileAppender(
    const tstring& filename_,
    std::ios_base::openmode mode_,
    bool immediateFlush_,
    bool createDirs_)
    : FileAppenderBase(filename_, mode_, immediateFlush_, createDirs_)
{
    init();
}


FileAppender::FileAppender(
    const Properties& props,
    std::ios_base::openmode mode_)
    : FileAppenderBase(props, mode_)
{
    init();
}

FileAppender::~FileAppender()
{
    destructorImpl();
}

///////////////////////////////////////////////////////////////////////////////
// FileAppender protected methods
///////////////////////////////////////////////////////////////////////////////

void
FileAppender::init()
{
    if (filename.empty())
    {
        getErrorHandler()->error( LOG4CPLUS_TEXT("Invalid filename") );
        return;
    }

    FileAppenderBase::init();
}

///////////////////////////////////////////////////////////////////////////////
// RollingFileAppender ctors and dtor
///////////////////////////////////////////////////////////////////////////////

RollingFileAppender::RollingFileAppender(const tstring& filename_,
    long maxFileSize_, int maxBackupIndex_, bool immediateFlush_,
    bool createDirs_)
    : FileAppender(filename_, std::ios_base::app, immediateFlush_, createDirs_)
{
    init(maxFileSize_, maxBackupIndex_);
}


RollingFileAppender::RollingFileAppender(const Properties& properties)
    : FileAppender(properties, std::ios_base::app)
{
    long tmpMaxFileSize = DEFAULT_ROLLING_LOG_SIZE;
    int tmpMaxBackupIndex = 1;
    tstring tmp (
        helpers::toUpper (
            properties.getProperty (LOG4CPLUS_TEXT ("MaxFileSize"))));
    if (! tmp.empty ())
    {
        tmpMaxFileSize = std::atoi(LOG4CPLUS_TSTRING_TO_STRING(tmp).c_str());
        if (tmpMaxFileSize != 0)
        {
            tstring::size_type const len = tmp.length();
            if (len > 2
                && tmp.compare (len - 2, 2, LOG4CPLUS_TEXT("MB")) == 0)
                tmpMaxFileSize *= (1024 * 1024); // convert to megabytes
            else if (len > 2
                && tmp.compare (len - 2, 2, LOG4CPLUS_TEXT("KB")) == 0)
                tmpMaxFileSize *= 1024; // convert to kilobytes
        }
    }

    properties.getInt (tmpMaxBackupIndex, LOG4CPLUS_TEXT("MaxBackupIndex"));

    init(tmpMaxFileSize, tmpMaxBackupIndex);
}


void
RollingFileAppender::init(long maxFileSize_, int maxBackupIndex_)
{
    if (maxFileSize_ < MINIMUM_ROLLING_LOG_SIZE)
    {
        tostringstream oss;
        oss << LOG4CPLUS_TEXT ("RollingFileAppender: MaxFileSize property")
            LOG4CPLUS_TEXT (" value is too small. Resetting to ")
            << MINIMUM_ROLLING_LOG_SIZE << ".";
        helpers::getLogLog ().warn (oss.str ());
        maxFileSize_ = MINIMUM_ROLLING_LOG_SIZE;
    }

    maxFileSize = maxFileSize_;
    maxBackupIndex = (std::max)(maxBackupIndex_, 1);
}


RollingFileAppender::~RollingFileAppender()
{
    destructorImpl();
}


///////////////////////////////////////////////////////////////////////////////
// RollingFileAppender protected methods
///////////////////////////////////////////////////////////////////////////////

// This method does not need to be locked since it is called by
// doAppend() which performs the locking
void
RollingFileAppender::append(const spi::InternalLoggingEvent& event)
{
    // Seek to the end of log file so that tellp() below returns the
    // right size.
    if (useLockFile)
        out.seekp (0, std::ios_base::end);

    // Rotate log file if needed before appending to it.
    if (out.tellp() > maxFileSize)
        rollover(true);

    FileAppender::append(event);

    // Rotate log file if needed after appending to it.
    if (out.tellp() > maxFileSize)
        rollover(true);
}


void
RollingFileAppender::rollover(bool alreadyLocked)
{
    helpers::LogLog & loglog = helpers::getLogLog();
    helpers::LockFileGuard guard;

    // Close the current file
    out.close();
    // Reset flags since the C++ standard specified that all the flags
    // should remain unchanged on a close.
    out.clear();

    if (useLockFile)
    {
        if (! alreadyLocked)
        {
            try
            {
                guard.attach_and_lock (*lockFile);
            }
            catch (std::runtime_error const &)
            {
                return;
            }
        }

        // Recheck the condition as there is a window where another
        // process can rollover the file before us.

        helpers::FileInfo fi;
        if (getFileInfo (&fi, filename) == -1
            || fi.size < maxFileSize)
        {
            // The file has already been rolled by another
            // process. Just reopen with the new file.

            // Open it up again.
            open (std::ios_base::out | std::ios_base::ate | std::ios_base::app);
            loglog_opening_result (loglog, out, filename);

            return;
        }
    }

    // If maxBackups <= 0, then there is no file renaming to be done.
    if (maxBackupIndex > 0)
    {
        rolloverFiles(filename, maxBackupIndex);

        // Rename fileName to fileName.1
        tstring target = filename + LOG4CPLUS_TEXT(".1");

        long ret;

#if defined (_WIN32)
        // Try to remove the target first. It seems it is not
        // possible to rename over existing file.
        ret = file_remove (target);
#endif

        loglog.debug (
            LOG4CPLUS_TEXT("Renaming file ")
            + filename
            + LOG4CPLUS_TEXT(" to ")
            + target);
        ret = file_rename (filename, target);
        loglog_renaming_result (loglog, filename, target, ret);
    }
    else
    {
        loglog.debug (filename + LOG4CPLUS_TEXT(" has no backups specified"));
    }

    // Open it up again in truncation mode
    open(std::ios::out | std::ios::trunc);
    loglog_opening_result (loglog, out, filename);
}


///////////////////////////////////////////////////////////////////////////////
// DailyRollingFileAppender ctors and dtor
///////////////////////////////////////////////////////////////////////////////

DailyRollingFileAppender::DailyRollingFileAppender(
    const tstring& filename_, DailyRollingFileSchedule schedule_,
    bool immediateFlush_, int maxBackupIndex_, bool createDirs_,
    bool rollOnClose_, const tstring& datePattern_)
    : FileAppender(filename_, std::ios_base::app, immediateFlush_, createDirs_)
    , maxBackupIndex(maxBackupIndex_)
    , rollOnClose(rollOnClose_)
    , datePattern(datePattern_)
{
    init(schedule_);
}



DailyRollingFileAppender::DailyRollingFileAppender(
    const Properties& properties)
    : FileAppender(properties, std::ios_base::app)
    , maxBackupIndex(10)
    , rollOnClose(true)
{
    DailyRollingFileSchedule theSchedule = DAILY;
    tstring scheduleStr (helpers::toUpper (
        properties.getProperty (LOG4CPLUS_TEXT ("Schedule"))));

    if(scheduleStr == LOG4CPLUS_TEXT("MONTHLY"))
        theSchedule = MONTHLY;
    else if(scheduleStr == LOG4CPLUS_TEXT("WEEKLY"))
        theSchedule = WEEKLY;
    else if(scheduleStr == LOG4CPLUS_TEXT("DAILY"))
        theSchedule = DAILY;
    else if(scheduleStr == LOG4CPLUS_TEXT("TWICE_DAILY"))
        theSchedule = TWICE_DAILY;
    else if(scheduleStr == LOG4CPLUS_TEXT("HOURLY"))
        theSchedule = HOURLY;
    else if(scheduleStr == LOG4CPLUS_TEXT("MINUTELY"))
        theSchedule = MINUTELY;
    else {
        helpers::getLogLog().warn(
            LOG4CPLUS_TEXT("DailyRollingFileAppender::ctor()")
            LOG4CPLUS_TEXT("- \"Schedule\" not valid: ")
            + properties.getProperty(LOG4CPLUS_TEXT("Schedule")));
        theSchedule = DAILY;
    }

    properties.getBool (rollOnClose, LOG4CPLUS_TEXT("RollOnClose"));
    properties.getString (datePattern, LOG4CPLUS_TEXT("DatePattern"));
    properties.getInt (maxBackupIndex, LOG4CPLUS_TEXT("MaxBackupIndex"));

    init(theSchedule);
}


namespace
{


static
Time
round_time (Time const & t_, helpers::chrono::seconds seconds)
{
    time_t const t = helpers::to_time_t (t_);
    return helpers::from_time_t (
        t - static_cast<time_t>(std::fmod (
                static_cast<double>(t),
                static_cast<double>(seconds.count ()))));
}


static
Time
round_time_and_add (Time const & t, helpers::chrono::seconds const & seconds)
{
    return round_time (t, seconds) + seconds;
}


static
std::chrono::seconds
local_time_offset (Time const & t)
{
    tm time_local, time_gmt;

    helpers::localTime (&time_local, t);
    helpers::gmTime (&time_gmt, t);

    Time t2 = helpers::from_struct_tm (&time_local);
    Time t3 = helpers::from_struct_tm (&time_gmt);

    return std::chrono::duration_cast<std::chrono::seconds> (t2 - t3);
}


static
Time
adjust_for_time_zone (Time const & t, std::chrono::seconds const & tzoffset)
{
    return helpers::time_cast (t - tzoffset);
}


} // namespace


void
DailyRollingFileAppender::init(DailyRollingFileSchedule sch)
{
    this->schedule = sch;
    Time now = helpers::truncate_fractions (helpers::now ());
    scheduledFilename = getFilename(now);
    nextRolloverTime = calculateNextRolloverTime(now);
}



DailyRollingFileAppender::~DailyRollingFileAppender()
{
    destructorImpl();
}




///////////////////////////////////////////////////////////////////////////////
// DailyRollingFileAppender public methods
///////////////////////////////////////////////////////////////////////////////

void
DailyRollingFileAppender::close()
{
    if (rollOnClose)
        rollover();
    FileAppender::close();
}



///////////////////////////////////////////////////////////////////////////////
// DailyRollingFileAppender protected methods
///////////////////////////////////////////////////////////////////////////////

// This method does not need to be locked since it is called by
// doAppend() which performs the locking
void
DailyRollingFileAppender::append(const spi::InternalLoggingEvent& event)
{
    if(event.getTimestamp() >= nextRolloverTime) {
        rollover(true);
    }

    FileAppender::append(event);
}



void
DailyRollingFileAppender::rollover(bool alreadyLocked)
{
    helpers::LockFileGuard guard;

    if (useLockFile && ! alreadyLocked)
    {
        try
        {
            guard.attach_and_lock (*lockFile);
        }
        catch (std::runtime_error const &)
        {
            return;
        }
    }

    // Close the current file
    out.close();
    // reset flags since the C++ standard specified that all the flags
    // should remain unchanged on a close
    out.clear();

    // If we've already rolled over this time period, we'll make sure that we
    // don't overwrite any of those previous files.
    // E.g. if "log.2009-11-07.1" already exists we rename it
    // to "log.2009-11-07.2", etc.
    rolloverFiles(scheduledFilename, maxBackupIndex);

    // Do not overwriet the newest file either, e.g. if "log.2009-11-07"
    // already exists rename it to "log.2009-11-07.1"
    tostringstream backup_target_oss;
    backup_target_oss << scheduledFilename << LOG4CPLUS_TEXT(".") << 1;
    tstring backupTarget = backup_target_oss.str();

    helpers::LogLog & loglog = helpers::getLogLog();
    long ret;

#if defined (_WIN32)
    // Try to remove the target first. It seems it is not
    // possible to rename over existing file, e.g. "log.2009-11-07.1".
    ret = file_remove (backupTarget);
#endif

    // Rename e.g. "log.2009-11-07" to "log.2009-11-07.1".
    ret = file_rename (scheduledFilename, backupTarget);
    loglog_renaming_result (loglog, scheduledFilename, backupTarget, ret);

#if defined (_WIN32)
    // Try to remove the target first. It seems it is not
    // possible to rename over existing file, e.g. "log.2009-11-07".
    ret = file_remove (scheduledFilename);
#endif

    // Rename filename to scheduledFilename,
    // e.g. rename "log" to "log.2009-11-07".
    loglog.debug(
        LOG4CPLUS_TEXT("Renaming file ")
        + filename
        + LOG4CPLUS_TEXT(" to ")
        + scheduledFilename);
    ret = file_rename (filename, scheduledFilename);
    loglog_renaming_result (loglog, filename, scheduledFilename, ret);

    // Open a new file, e.g. "log".
    open(std::ios::out | std::ios::trunc);
    loglog_opening_result (loglog, out, filename);

    // Calculate the next rollover time
    log4cplus::helpers::Time now = helpers::now ();
    if (now >= nextRolloverTime)
    {
        scheduledFilename = getFilename(now);
        nextRolloverTime = calculateNextRolloverTime(now);
    }
}


static
Time
calculateNextRolloverTime(const Time& t, DailyRollingFileSchedule schedule)
{
    namespace chrono = helpers::chrono;

    struct tm next;
    switch(schedule)
    {
    case MONTHLY:
    {
        helpers::localTime (&next, t);
        next.tm_mon += 1;
        next.tm_mday = 1; // Round up to next month start
        next.tm_hour = 0;
        next.tm_min = 0;
        next.tm_sec = 0;
        next.tm_isdst = -1;

        Time ret;
        try
        {
            ret = helpers::from_struct_tm (&next);
        }
        catch (std::runtime_error const & e)
        {
            helpers::getLogLog().error(
                LOG4CPLUS_TEXT("calculateNextRolloverTime()-")
                LOG4CPLUS_TEXT(" from_struct_tm() returned error: ")
                + LOG4CPLUS_C_STR_TO_TSTRING (e.what ()));
            // Set next rollover to 31 days in future.
            ret = round_time (t, chrono::seconds (24 * 60 * 60))
                + chrono::seconds (2678400);
        }
        return ret;
    }

    case WEEKLY:
    {
        helpers::localTime (&next, t);
        // Round up to next week
        next.tm_mday += (7 - next.tm_wday + 1);
        next.tm_hour = 0;
        next.tm_min = 0;
        next.tm_sec = 0;
        next.tm_isdst = -1;

        Time ret;
        try
        {
            ret = helpers::from_struct_tm (&next);
        }
        catch (std::runtime_error const & e)
        {
            helpers::getLogLog().error(
                LOG4CPLUS_TEXT("calculateNextRolloverTime()-")
                LOG4CPLUS_TEXT(" from_struct_tm() returned error: ")
                + LOG4CPLUS_C_STR_TO_TSTRING (e.what ()));
            // Set next rollover to 7 days in future.
            ret = round_time (t, chrono::seconds (24 * 60 * 60))
                + chrono::seconds (7 * 24 * 60 * 60);
        }
        return ret;
    }

    default:
        helpers::getLogLog ().error (
            LOG4CPLUS_TEXT ("calculateNextRolloverTime()-")
            LOG4CPLUS_TEXT (" unhandled or invalid schedule value"));
        // Fall through.

    case DAILY:
    {
        helpers::localTime(&next, t);
        next.tm_mday += 1;
        next.tm_hour = 0;
        next.tm_min = 0;
        next.tm_sec = 0;
        next.tm_isdst = -1;

        Time ret;
        try
        {
            ret = helpers::from_struct_tm (&next);
        }
        catch (std::runtime_error const & e)
        {
            helpers::getLogLog().error(
                LOG4CPLUS_TEXT("calculateNextRolloverTime()-")
                LOG4CPLUS_TEXT(" from_struct_tm() returned error: ")
                + LOG4CPLUS_C_STR_TO_TSTRING (e.what ()));
            // Set next rollover to 24 hours in future.
            ret = round_time (t, chrono::seconds (60 * 60))
                + chrono::seconds (24 * 60 * 60);
        }

        return ret;
    }

    case TWICE_DAILY:
    {
        helpers::localTime(&next, t);
        if (next.tm_hour < 12)
            next.tm_hour = 12;
        else
            next.tm_hour = 24;
        next.tm_min = 0;
        next.tm_sec = 0;
        next.tm_isdst = -1;

        Time ret;
        try
        {
            ret = helpers::from_struct_tm (&next);
        }
        catch (std::runtime_error const & e)
        {
            helpers::getLogLog().error(
                LOG4CPLUS_TEXT("calculateNextRolloverTime()-")
                LOG4CPLUS_TEXT(" from_struct_tm() returned error: ")
                + LOG4CPLUS_C_STR_TO_TSTRING (e.what ()));
            // Set next rollover to 12 hours in future.
            ret = round_time (t, chrono::seconds (60 * 60))
                + chrono::seconds (12 * 60 * 60);
        }

        return ret;
    }

    case HOURLY:
    {
        helpers::localTime(&next, t);
        next.tm_hour += 1;
        next.tm_min = 0;
        next.tm_sec = 0;
        next.tm_isdst = -1;

        Time ret;
        try
        {
            ret = helpers::from_struct_tm (&next);
        }
        catch (std::runtime_error const & e)
        {
            helpers::getLogLog().error(
                LOG4CPLUS_TEXT("calculateNextRolloverTime()-")
                LOG4CPLUS_TEXT(" from_struct_tm() returned error: ")
                + LOG4CPLUS_C_STR_TO_TSTRING (e.what ()));
            // Set next rollover to 60 minutes in future.
            ret = round_time (t, chrono::seconds (60 * 60))
                + chrono::seconds (60 * 60);
        }

        return ret;
    }

    case MINUTELY:
        return round_time_and_add (t, chrono::seconds (60));
    };
}


Time
DailyRollingFileAppender::calculateNextRolloverTime(const Time& t) const
{
    return helpers::truncate_fractions (
        log4cplus::calculateNextRolloverTime (t, schedule));
}


tstring
DailyRollingFileAppender::getFilename(const Time& t) const
{
    tchar const * pattern = nullptr;
    if (datePattern.empty())
    {
        switch (schedule)
        {
        case MONTHLY:
            pattern = LOG4CPLUS_TEXT("%Y-%m");
            break;

        case WEEKLY:
            pattern = LOG4CPLUS_TEXT("%Y-%W");
            break;

        default:
            helpers::getLogLog ().error (
                LOG4CPLUS_TEXT ("DailyRollingFileAppender::getFilename()-")
                LOG4CPLUS_TEXT (" invalid schedule value"));
            // Fall through.

        case DAILY:
            pattern = LOG4CPLUS_TEXT("%Y-%m-%d");
            break;

        case TWICE_DAILY:
            pattern = LOG4CPLUS_TEXT("%Y-%m-%d-%p");
            break;

        case HOURLY:
            pattern = LOG4CPLUS_TEXT("%Y-%m-%d-%H");
            break;

        case MINUTELY:
            pattern = LOG4CPLUS_TEXT("%Y-%m-%d-%H-%M");
            break;
        };
    }
    else
        pattern = datePattern.c_str();

    tstring result (filename);
    result += LOG4CPLUS_TEXT(".");
    result += helpers::getFormattedTime(pattern, t, false);
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// TimeBasedRollingFileAppender utility functions
///////////////////////////////////////////////////////////////////////////////

static tstring
preprocessDateTimePattern(const tstring& pattern, DailyRollingFileSchedule& schedule)
{
    // Example: "yyyy-MM-dd HH:mm:ss,aux"
    // Example with space(s) between the ',' and 'aux': "yyyy-MM-dd HH:mm:ss, aux"
    // Patterns from java.text.SimpleDateFormat not implemented here: Y, F, k, K, S, X

    tostringstream result;

    size_t aux_len = 0;
    auto pattern_length = pattern.length();
    if (pattern_length >= 4 && pattern.find(LOG4CPLUS_TEXT("aux"), pattern_length-3) == pattern_length-3) {
        auto const comma_pos = pattern.rfind(LOG4CPLUS_TEXT(","));
        if (comma_pos != tstring::npos) {
            auto const comma_to_aux_len = pattern_length - 4 - comma_pos;
            if (comma_to_aux_len == 0) {
                aux_len = 4;
            }
            else if (comma_to_aux_len > 0) {
                auto const space_str = pattern.substr(comma_pos+1, comma_to_aux_len);
                if (space_str == tstring(comma_to_aux_len, ' ')) {
                    aux_len = 4 + comma_to_aux_len;
                }
            }
            pattern_length -= aux_len;
        }
    }

    bool has_week = false, has_day = false, has_hour = false, has_minute = false;

    for (size_t i = 0; i < pattern_length; )
    {
        tchar c = pattern[i];
        size_t end_pos = pattern.find_first_not_of(c, i);
        size_t len = (end_pos == tstring::npos ? pattern.length() : end_pos) - i;

        switch (c)
        {
        case LOG4CPLUS_TEXT('y'): // Year number
            if (len == 2)
                result << LOG4CPLUS_TEXT("%y");
            else if (len == 4)
                result << LOG4CPLUS_TEXT("%Y");
            break;
        case LOG4CPLUS_TEXT('Y'): // Week year
            if (len == 2)
                result << LOG4CPLUS_TEXT("%g");
            else if (len == 4)
                result << LOG4CPLUS_TEXT("%G");
            break;
        case LOG4CPLUS_TEXT('M'): // Month in year
            if (len == 2)
                result << LOG4CPLUS_TEXT("%m");
            else if (len == 3)
                result << LOG4CPLUS_TEXT("%b");
            else if (len > 3)
                result << LOG4CPLUS_TEXT("%B");
            break;
        case LOG4CPLUS_TEXT('w'): // Week in year
            if (len == 2) {
                result << LOG4CPLUS_TEXT("%W");
                has_week = true;
            }
            break;
        case LOG4CPLUS_TEXT('D'): // Day in year
            if (len == 3) {
                result << LOG4CPLUS_TEXT("%j");
                has_day = true;
            }
            break;
        case LOG4CPLUS_TEXT('d'): // Day in month
            if (len == 2) {
                result << LOG4CPLUS_TEXT("%d");
                has_day = true;
            }
            break;
        case LOG4CPLUS_TEXT('E'): // Day name in week
            if (len == 3) {
                result << LOG4CPLUS_TEXT("%a");
                has_day = true;
            } else if (len > 3) {
                result << LOG4CPLUS_TEXT("%A");
                has_day = true;
            }
            break;
        case LOG4CPLUS_TEXT('u'): // Day number of week
            if (len == 1) {
                result << LOG4CPLUS_TEXT("%u");
                has_day = true;
            }
            break;
        case LOG4CPLUS_TEXT('a'): // AM/PM marker
            if (len == 2)
                result << LOG4CPLUS_TEXT("%p");
            break;
        case LOG4CPLUS_TEXT('H'): // Hour in day
            if (len == 2) {
                result << LOG4CPLUS_TEXT("%H");
                has_hour = true;
            }
            break;
        case LOG4CPLUS_TEXT('h'): // Hour in am/pm (01-12)
            if (len == 2) {
                result << LOG4CPLUS_TEXT("%I");
                has_hour = true;
            }
            break;
        case LOG4CPLUS_TEXT('m'): // Minute in hour
            if (len == 2) {
                result << LOG4CPLUS_TEXT("%M");
                has_minute = true;
            }
            break;
        case LOG4CPLUS_TEXT('s'): // Second in minute
            if (len == 2)
                result << LOG4CPLUS_TEXT("%S");
            break;
        case LOG4CPLUS_TEXT('z'): // Time zone name
            if (len == 1)
                result << LOG4CPLUS_TEXT("%Z");
            break;
        case LOG4CPLUS_TEXT('Z'): // Time zone offset
            if (len == 1)
                result << LOG4CPLUS_TEXT("%z");
            break;
        default:
            result << c;
        }

        i += len;
    }

    if (aux_len == 0)
    {
        if (has_minute)
            schedule = MINUTELY;
        else if (has_hour)
            schedule = HOURLY;
        else if (has_day)
            schedule = DAILY;
        else if (has_week)
            schedule = WEEKLY;
        else
            schedule = MONTHLY;
    }

    return result.str();
}

static tstring
preprocessFilenamePattern(const tstring& pattern, DailyRollingFileSchedule& schedule)
{
    tostringstream result;

    for (size_t i = 0; i < pattern.length(); )
    {
        tchar c = pattern[i];

        if (c == LOG4CPLUS_TEXT('%') &&
            i < pattern.length()-1 &&
            pattern[i+1] == LOG4CPLUS_TEXT('d'))
        {
            if (i < pattern.length()-2 && pattern[i+2] == LOG4CPLUS_TEXT('{'))
            {
                size_t closingBracketPos = pattern.find(LOG4CPLUS_TEXT("}"), i+2);
                if (closingBracketPos == std::string::npos)
                {
                    break; // Malformed conversion specifier
                }
                else
                {
                    result << preprocessDateTimePattern(pattern.substr(i+3, closingBracketPos-(i+3)), schedule);
                    i = closingBracketPos + 1;
                }
            }
            else
            {
                // Default conversion specifier
                result << preprocessDateTimePattern(LOG4CPLUS_TEXT("yyyy-MM-dd"), schedule);
                i += 2;
            }
        }
        else
        {
            result << c;
            i += 1;
        }
    }

    return result.str();
}


///////////////////////////////////////////////////////////////////////////////
// TimeBasedRollingFileAppender ctors and dtor
///////////////////////////////////////////////////////////////////////////////

TimeBasedRollingFileAppender::TimeBasedRollingFileAppender(
    const tstring& filename_,
    const tstring& filenamePattern_,
    int maxHistory_,
    bool cleanHistoryOnStart_,
    bool immediateFlush_,
    bool createDirs_,
    bool rollOnClose_)
    : FileAppenderBase(filename_, std::ios_base::app, immediateFlush_, createDirs_)
    , filenamePattern(filenamePattern_)
    , schedule(DAILY)
    , maxHistory(maxHistory_)
    , cleanHistoryOnStart(cleanHistoryOnStart_)
    , rollOnClose(rollOnClose_)
{
    filenamePattern = preprocessFilenamePattern(filenamePattern, schedule);
    init();
}

TimeBasedRollingFileAppender::TimeBasedRollingFileAppender(
    const log4cplus::helpers::Properties& properties)
    : FileAppenderBase(properties, std::ios_base::app)
    , filenamePattern(LOG4CPLUS_TEXT("%d.log"))
    , schedule(DAILY)
    , maxHistory(10)
    , cleanHistoryOnStart(false)
    , rollOnClose(true)
{
    filenamePattern = properties.getProperty(LOG4CPLUS_TEXT("FilenamePattern"));
    properties.getInt(maxHistory, LOG4CPLUS_TEXT("MaxHistory"));
    properties.getBool(cleanHistoryOnStart, LOG4CPLUS_TEXT("CleanHistoryOnStart"));
    properties.getBool(rollOnClose, LOG4CPLUS_TEXT("RollOnClose"));
    filenamePattern = preprocessFilenamePattern(filenamePattern, schedule);

    init();
}

TimeBasedRollingFileAppender::~TimeBasedRollingFileAppender()
{
    destructorImpl();
}

///////////////////////////////////////////////////////////////////////////////
// TimeBasedRollingFileAppender protected methods
///////////////////////////////////////////////////////////////////////////////

void
TimeBasedRollingFileAppender::init()
{
    if (filenamePattern.empty())
    {
        getErrorHandler()->error( LOG4CPLUS_TEXT("Invalid filename/filenamePattern values") );
        return;
    }

    FileAppenderBase::init();

    Time now = timeBasedAppenderNow ();
    nextRolloverTime = calculateNextRolloverTime(now);

    if (LOG4CPLUS_UNLIKELY(cleanHistoryOnStart))
    {
        clean(now + maxHistory*getRolloverPeriodDuration());
    }
    else
    {
        clean(now);
    }

    lastHeartBeat = now;
}

void
TimeBasedRollingFileAppender::append(const spi::InternalLoggingEvent& event)
{
    if(event.getTimestamp() >= nextRolloverTime) {
        rollover(true);
    }

    FileAppenderBase::append(event);
}

void
TimeBasedRollingFileAppender::open(std::ios_base::openmode mode)
{
    scheduledFilename = helpers::getFormattedTime(filenamePattern, timeBasedAppenderNow (), false);
    tstring currentFilename = filename.empty () ? scheduledFilename : filename;

    if (createDirs)
        internal::make_dirs (currentFilename);

    out.open(LOG4CPLUS_FSTREAM_PREFERED_FILE_NAME(currentFilename).c_str(), mode);
    if(!out.good())
    {
        getErrorHandler()->error(LOG4CPLUS_TEXT("Unable to open file: ") + currentFilename);
        return;
    }
    helpers::getLogLog().debug(LOG4CPLUS_TEXT("Just opened file: ") + currentFilename);
}

void
TimeBasedRollingFileAppender::close()
{
    if (rollOnClose)
        rollover();
    FileAppenderBase::close();
}

void
TimeBasedRollingFileAppender::rollover(bool alreadyLocked)
{
    helpers::LockFileGuard guard;

    if (useLockFile && ! alreadyLocked)
    {
        try
        {
            guard.attach_and_lock (*lockFile);
        }
        catch (std::runtime_error const &)
        {
            return;
        }
    }

    // Close the current file
    out.close();
    // reset flags since the C++ standard specified that all the flags
    // should remain unchanged on a close
    out.clear();

    if (! filename.empty () && filename != scheduledFilename)
    {
        helpers::LogLog & loglog = helpers::getLogLog();
        long ret;

#if defined (_WIN32)
        // Try to remove the target first. It seems it is not
        // possible to rename over existing file.
        ret = file_remove (scheduledFilename);
#endif

        loglog.debug(
            LOG4CPLUS_TEXT("Renaming file ")
            + filename
            + LOG4CPLUS_TEXT(" to ")
            + scheduledFilename);
        ret = file_rename (filename, scheduledFilename);
        loglog_renaming_result (loglog, filename, scheduledFilename, ret);
    }

    Time now = timeBasedAppenderNow ();
    clean(now);

    open(std::ios::out | (filename.empty () ? std::ios::app : std::ios::trunc));

    nextRolloverTime = calculateNextRolloverTime(now);
}

void
TimeBasedRollingFileAppender::clean(Time time)
{
    Time::duration interval = std::chrono::hours{31*24}; // ~1 month
    if (lastHeartBeat != Time{})
    {
        interval = time - lastHeartBeat + std::chrono::seconds{1};
    }

    Time::duration period = getRolloverPeriodDuration();
    long periods = long(interval.count () / period.count ());
    // A partial interval can cross an additional archive period boundary.
    if (interval % period > Time::duration::zero ())
        ++periods;

    helpers::LogLog & loglog = helpers::getLogLog();
    for (long i = 0; i < periods; i++)
    {
        long periodToRemove = (-maxHistory - 1) - i;
        Time timeToRemove = time + periodToRemove * period;
        tstring filenameToRemove = helpers::getFormattedTime(filenamePattern, timeToRemove, false);
        loglog.debug(LOG4CPLUS_TEXT("Removing file ") + filenameToRemove);
        file_remove(filenameToRemove);
    }

    lastHeartBeat = time;
}

Time::duration
TimeBasedRollingFileAppender::getRolloverPeriodDuration() const
{
    switch (schedule)
    {
    case MONTHLY:
        return std::chrono::hours{31*24};
    case WEEKLY:
        return std::chrono::hours{7*24};
    default:
        helpers::getLogLog ().error (
            LOG4CPLUS_TEXT ("TimeBasedRollingFileAppender::getRolloverPeriodDuration()-")
            LOG4CPLUS_TEXT (" invalid schedule value"));
        // Fall through.
    case DAILY:
        return std::chrono::hours{24};
    case HOURLY:
        return std::chrono::hours{1};
    case MINUTELY:
        return std::chrono::minutes{1};
    }
}

Time
TimeBasedRollingFileAppender::calculateNextRolloverTime(const Time& t) const
{
    return helpers::truncate_fractions (
        log4cplus::calculateNextRolloverTime (t, schedule));
}

#if defined (LOG4CPLUS_WITH_UNIT_TESTS)
namespace
{

Time
timeBasedTestDate (int day)
{
    std::tm date {};
    date.tm_year = 125;
    date.tm_mon = 10;
    date.tm_mday = day;
    date.tm_hour = 12;
    date.tm_isdst = -1;
    return helpers::from_time_t (std::mktime (&date));
}

struct TimeBasedTestClock
{
    Time now = timeBasedTestDate (14);
    Time const * previous = timeBasedAppenderTestTime;

    TimeBasedTestClock ()
    {
        timeBasedAppenderTestTime = &now;
    }

    ~TimeBasedTestClock ()
    {
        timeBasedAppenderTestTime = previous;
    }
};

struct TimeBasedTestDirectory
{
    tstring path;
    std::vector<tstring> files;
    std::vector<tstring> directories;

    TimeBasedTestDirectory ()
    {
        if ((! internal::get_env_var (path, LOG4CPLUS_TEXT ("TMPDIR"))
            && ! internal::get_env_var (path, LOG4CPLUS_TEXT ("TEMP"))
            && ! internal::get_env_var (path, LOG4CPLUS_TEXT ("TMP")))
            || path.empty ())
            path = LOG4CPLUS_TEXT (".");
        path += LOG4CPLUS_TEXT ("/log4cplus-rollover-")
            + LOG4CPLUS_STRING_TO_TSTRING (std::to_string (
                internal::get_process_id ()))
            + LOG4CPLUS_TEXT ("-")
            + LOG4CPLUS_STRING_TO_TSTRING (std::to_string (
                std::chrono::steady_clock::now ().time_since_epoch ().count ()));
#if defined (_WIN32) && defined (UNICODE)
        int result = _wmkdir (path.c_str ());
#elif defined (_WIN32)
        int result = _mkdir (path.c_str ());
#else
        int result = mkdir (LOG4CPLUS_TSTRING_TO_STRING (path).c_str (), 0700);
#endif
        CATCH_REQUIRE (result == 0);
        directories.push_back (path);
    }

    ~TimeBasedTestDirectory ()
    {
        for (auto const & name : files)
            file_remove (name);
        for (auto i = directories.rbegin (); i != directories.rend (); ++i)
        {
#if defined (_WIN32) && defined (UNICODE)
            _wrmdir (i->c_str ());
#elif defined (_WIN32)
            _rmdir (i->c_str ());
#else
            rmdir (LOG4CPLUS_TSTRING_TO_STRING ((*i)).c_str ());
#endif
        }
    }

    tstring file (tstring const & relative)
    {
        tstring result = path + LOG4CPLUS_TEXT ("/") + relative;
        files.push_back (result);
        for (size_t i = 0; (i = relative.find (LOG4CPLUS_TEXT ('/'), i))
            != tstring::npos; ++i)
        {
            auto const directory = path + LOG4CPLUS_TEXT ("/")
                + relative.substr (0, i);
            if (std::find (directories.begin (), directories.end (), directory)
                == directories.end ())
                directories.push_back (directory);
        }
        return result;
    }

    static bool exists (tstring const & name)
    {
        helpers::FileInfo info;
        return helpers::getFileInfo (&info, name) == 0;
    }

    static std::string read (tstring const & name)
    {
        std::ifstream input (LOG4CPLUS_TSTRING_TO_STRING (name).c_str ());
        CATCH_REQUIRE (input.good ());
        return std::string (std::istreambuf_iterator<char> (input),
            std::istreambuf_iterator<char> ());
    }

    static void write (tstring const & name, char const * contents)
    {
        std::ofstream output (LOG4CPLUS_TSTRING_TO_STRING (name).c_str ());
        output << contents;
        CATCH_REQUIRE (output.good ());
    }
};

class TimeBasedTestAppender : public TimeBasedRollingFileAppender
{
public:
    using TimeBasedRollingFileAppender::TimeBasedRollingFileAppender;
    using TimeBasedRollingFileAppender::close;
    using TimeBasedRollingFileAppender::rollover;

    void log (tstring const & message, Time time)
    {
        spi::InternalLoggingEvent event (LOG4CPLUS_TEXT ("rollover-test"),
            INFO_LOG_LEVEL, LOG4CPLUS_TEXT (""), MappedDiagnosticContextMap {},
            message, LOG4CPLUS_TEXT ("thread"), LOG4CPLUS_TEXT (""), time,
            LOG4CPLUS_TEXT (""), 0);
        doAppend (event);
    }

    void failStream ()
    {
        out.setstate (std::ios_base::badbit);
        reopenDelay = 0;
    }
};

Properties
timeBasedTestProperties (tstring const & pattern)
{
    Properties properties;
    properties.setProperty (LOG4CPLUS_TEXT ("FilenamePattern"), pattern);
    properties.setProperty (LOG4CPLUS_TEXT ("MaxHistory"), LOG4CPLUS_TEXT ("365"));
    properties.setProperty (LOG4CPLUS_TEXT ("RollOnClose"), LOG4CPLUS_TEXT ("false"));
    properties.setProperty (LOG4CPLUS_TEXT ("CreateDirs"), LOG4CPLUS_TEXT ("true"));
    properties.setProperty (LOG4CPLUS_TEXT ("ReopenDelay"), LOG4CPLUS_TEXT ("0"));
    return properties;
}

void
timeBasedTestLayout (TimeBasedTestAppender & appender)
{
    appender.setLayout (std::unique_ptr<Layout> (
        new PatternLayout (LOG4CPLUS_TEXT ("%m%n"))));
}

class TimeBasedTestErrorHandler : public ErrorHandler
{
public:
    std::vector<tstring> errors;

    void error (tstring const & message) override
    {
        errors.push_back (message);
    }

    void reset () override {}
};

} // namespace

CATCH_TEST_CASE ("TimeBasedRollingFileAppender preserves both filename modes",
    "[appender][timebased-rollover]")
{
    TimeBasedTestClock clock;
    TimeBasedTestDirectory directory;
    auto const pattern = directory.path
        + LOG4CPLUS_TEXT ("/archive-%d{yyyyMMdd}.log");
    auto properties = timeBasedTestProperties (pattern);
    auto const current = directory.file (LOG4CPLUS_TEXT ("current.log"));
    auto const first = directory.file (LOG4CPLUS_TEXT ("archive-20251114.log"));
    auto const second = directory.file (LOG4CPLUS_TEXT ("archive-20251115.log"));
    auto const third = directory.file (LOG4CPLUS_TEXT ("archive-20251116.log"));
    bool fixed = false;
    bool direct = false;
    CATCH_SECTION ("properties without File") {}
    CATCH_SECTION ("properties with empty File")
    {
        properties.setProperty (LOG4CPLUS_TEXT ("File"), LOG4CPLUS_TEXT (""));
    }
    CATCH_SECTION ("properties with fixed File") { fixed = true; }
    CATCH_SECTION ("direct construction without fixed file") { direct = true; }
    CATCH_SECTION ("direct construction with fixed file")
    {
        direct = true;
        fixed = true;
    }
    if (fixed)
        properties.setProperty (LOG4CPLUS_TEXT ("File"), current);
    std::unique_ptr<TimeBasedTestAppender> appender;
    if (direct)
        appender.reset (new TimeBasedTestAppender (
            fixed ? current : tstring (), pattern, 365, false, true, true, false));
    else
        appender.reset (new TimeBasedTestAppender (properties));
    timeBasedTestLayout (*appender);

    appender->log (LOG4CPLUS_TEXT ("one"), clock.now);
    CATCH_CHECK (directory.read (fixed ? current : first) == "one\n");
    CATCH_CHECK (directory.exists (first) == ! fixed);
    clock.now = timeBasedTestDate (15);
    appender->log (LOG4CPLUS_TEXT ("two"), clock.now);
    CATCH_CHECK (directory.read (first) == "one\n");
    CATCH_CHECK (directory.read (fixed ? current : second) == "two\n");
    CATCH_CHECK (directory.exists (second) == ! fixed);
    clock.now = timeBasedTestDate (16);
    appender->log (LOG4CPLUS_TEXT ("three"), clock.now);
    CATCH_CHECK (directory.read (first) == "one\n");
    CATCH_CHECK (directory.read (second) == "two\n");
    CATCH_CHECK (directory.read (fixed ? current : third) == "three\n");
    CATCH_CHECK (directory.exists (third) == ! fixed);
}

CATCH_TEST_CASE ("TimeBasedRollingFileAppender preserves existing pattern files",
    "[appender][timebased-rollover]")
{
    TimeBasedTestClock clock;
    TimeBasedTestDirectory directory;
    auto properties = timeBasedTestProperties (directory.path
        + LOG4CPLUS_TEXT ("/archive-%d{yyyyMMdd}.log"));
    properties.setProperty (LOG4CPLUS_TEXT ("RollOnClose"), LOG4CPLUS_TEXT ("true"));
    auto const first = directory.file (LOG4CPLUS_TEXT ("archive-20251114.log"));
    auto const second = directory.file (LOG4CPLUS_TEXT ("archive-20251115.log"));
    TimeBasedTestAppender appender (properties);
    timeBasedTestLayout (appender);
    appender.log (LOG4CPLUS_TEXT ("one"), clock.now);

    CATCH_SECTION ("rollover appends to an existing destination")
    {
        directory.write (second, "previous\n");
        clock.now = timeBasedTestDate (15);
        appender.log (LOG4CPLUS_TEXT ("two"), clock.now);
        CATCH_CHECK (directory.read (first) == "one\n");
        CATCH_CHECK (directory.read (second) == "previous\ntwo\n");
    }
    CATCH_SECTION ("repeated rollover within the same period")
    {
        appender.rollover ();
        appender.rollover ();
        appender.log (LOG4CPLUS_TEXT ("two"), clock.now);
        CATCH_CHECK (directory.read (first) == "one\ntwo\n");
    }
    CATCH_SECTION ("RollOnClose preserves the current period")
    {
        appender.close ();
        CATCH_CHECK (directory.read (first) == "one\n");
    }
}

CATCH_TEST_CASE ("TimeBasedRollingFileAppender creates dated output directories",
    "[appender][timebased-rollover]")
{
    TimeBasedTestClock clock;
    TimeBasedTestDirectory directory;
    auto const first = directory.file (LOG4CPLUS_TEXT ("20251114/app.log"));
    auto const second = directory.file (LOG4CPLUS_TEXT ("20251115/app.log"));
    TimeBasedTestAppender appender (timeBasedTestProperties (directory.path
        + LOG4CPLUS_TEXT ("/%d{yyyyMMdd}/app.log")));
    timeBasedTestLayout (appender);
    appender.log (LOG4CPLUS_TEXT ("one"), clock.now);
    clock.now = timeBasedTestDate (15);
    appender.log (LOG4CPLUS_TEXT ("two"), clock.now);
    CATCH_CHECK (directory.read (first) == "one\n");
    CATCH_CHECK (directory.read (second) == "two\n");
}

CATCH_TEST_CASE ("TimeBasedRollingFileAppender reopens without losing messages",
    "[appender][timebased-rollover]")
{
    TimeBasedTestClock clock;
    TimeBasedTestDirectory directory;
    auto const first = directory.file (LOG4CPLUS_TEXT ("archive-20251114.log"));
    auto const second = directory.file (LOG4CPLUS_TEXT ("archive-20251115.log"));
    auto const current = directory.file (LOG4CPLUS_TEXT ("current.log"));
    auto properties = timeBasedTestProperties (directory.path
        + LOG4CPLUS_TEXT ("/archive-%d{yyyyMMdd}.log"));
    bool fixed = false;
    CATCH_SECTION ("pattern-only file") {}
    CATCH_SECTION ("fixed file") { fixed = true; }
    if (fixed)
        properties.setProperty (LOG4CPLUS_TEXT ("File"), current);
    TimeBasedTestAppender appender (properties);
    timeBasedTestLayout (appender);
    auto * errors = new TimeBasedTestErrorHandler;
    appender.setErrorHandler (std::unique_ptr<ErrorHandler> (errors));
    appender.log (LOG4CPLUS_TEXT ("one"), clock.now);
    appender.failStream ();
    appender.log (LOG4CPLUS_TEXT ("two"), clock.now);
    CATCH_CHECK (directory.read (fixed ? current : first) == "one\ntwo\n");
    clock.now = timeBasedTestDate (15);
    appender.log (LOG4CPLUS_TEXT ("three"), clock.now);
    appender.failStream ();
    appender.log (LOG4CPLUS_TEXT ("four"), clock.now);
    CATCH_CHECK (directory.read (first) == "one\ntwo\n");
    CATCH_CHECK (directory.read (fixed ? current : second) == "three\nfour\n");
    CATCH_CHECK (errors->errors.empty ());
}

CATCH_TEST_CASE ("TimeBasedRollingFileAppender reports the selected output path",
    "[appender][timebased-rollover]")
{
    TimeBasedTestClock clock;
    TimeBasedTestDirectory directory;
    auto const output = directory.file (
        LOG4CPLUS_TEXT ("missing/archive-20251114.log"));
    auto properties = timeBasedTestProperties (directory.path
        + LOG4CPLUS_TEXT ("/missing/archive-%d{yyyyMMdd}.log"));
    properties.setProperty (LOG4CPLUS_TEXT ("CreateDirs"), LOG4CPLUS_TEXT ("false"));
    TimeBasedTestAppender appender (properties);
    auto * errors = new TimeBasedTestErrorHandler;
    appender.setErrorHandler (std::unique_ptr<ErrorHandler> (errors));
    appender.log (LOG4CPLUS_TEXT ("one"), clock.now);
    CATCH_REQUIRE (errors->errors.size () == 2);
    CATCH_CHECK (errors->errors[0] == LOG4CPLUS_TEXT ("Unable to open file: ")
        + output);
    CATCH_CHECK (errors->errors[1] == LOG4CPLUS_TEXT ("file is not open"));
}

CATCH_TEST_CASE ("TimeBasedRollingFileAppender initially honors Append",
    "[appender][timebased-rollover]")
{
    TimeBasedTestClock clock;
    TimeBasedTestDirectory directory;
    auto const output = directory.file (LOG4CPLUS_TEXT ("archive-20251114.log"));
    directory.write (output, "previous\n");
    auto properties = timeBasedTestProperties (directory.path
        + LOG4CPLUS_TEXT ("/archive-%d{yyyyMMdd}.log"));
    bool append = true;
    CATCH_SECTION ("Append=true") {}
    CATCH_SECTION ("Append=false") { append = false; }
    properties.setProperty (LOG4CPLUS_TEXT ("Append"),
        append ? LOG4CPLUS_TEXT ("true") : LOG4CPLUS_TEXT ("false"));
    TimeBasedTestAppender appender (properties);
    timeBasedTestLayout (appender);
    appender.log (LOG4CPLUS_TEXT ("one"), clock.now);
    CATCH_CHECK (directory.read (output) == (append ? "previous\none\n" : "one\n"));
}

CATCH_TEST_CASE ("TimeBasedRollingFileAppender cleanup across partial periods",
    "[appender]")
{
    struct TestDirectory
    {
        tstring path;
        std::vector<tstring> files;

        TestDirectory ()
        {
            if ((! internal::get_env_var (path, LOG4CPLUS_TEXT ("TMPDIR"))
                && ! internal::get_env_var (path, LOG4CPLUS_TEXT ("TEMP"))
                && ! internal::get_env_var (path, LOG4CPLUS_TEXT ("TMP")))
                || path.empty ())
                path = LOG4CPLUS_TEXT (".");
            path += LOG4CPLUS_TEXT ("/log4cplus-clean-")
                + LOG4CPLUS_STRING_TO_TSTRING (std::to_string (
                    internal::get_process_id ()))
                + LOG4CPLUS_TEXT ("-")
                + LOG4CPLUS_STRING_TO_TSTRING (std::to_string (
                    std::chrono::steady_clock::now ().time_since_epoch ().count ()));
#if defined (_WIN32) && defined (UNICODE)
            int result = _wmkdir (path.c_str ());
#elif defined (_WIN32)
            int result = _mkdir (path.c_str ());
#else
            int result = mkdir (LOG4CPLUS_TSTRING_TO_STRING (path).c_str (), 0700);
#endif
            if (result != 0)
                throw std::runtime_error ("Cannot create cleanup test directory");
        }

        ~TestDirectory ()
        {
            for (auto const & file : files)
                file_remove (file);
#if defined (_WIN32) && defined (UNICODE)
            _wrmdir (path.c_str ());
#elif defined (_WIN32)
            _rmdir (path.c_str ());
#else
            rmdir (LOG4CPLUS_TSTRING_TO_STRING (path).c_str ());
#endif
        }

        tstring file (tstring const & name)
        {
            tstring result = path + LOG4CPLUS_TEXT ("/") + name;
            files.push_back (result);
            return result;
        }
    } directory;
    auto const exists = [] (tstring const & name) -> bool
    {
        helpers::FileInfo info;
        return helpers::getFileInfo (&info, name) == 0;
    };

    class TestAppender : public TimeBasedRollingFileAppender
    {
    public:
        using TimeBasedRollingFileAppender::TimeBasedRollingFileAppender;
        using TimeBasedRollingFileAppender::clean;
        using TimeBasedRollingFileAppender::lastHeartBeat;
    };

    int previous_seconds = 170; // 12:02:50
    int current_seconds = 310;  // 12:05:10
    CATCH_SECTION ("several minute boundaries in a partial interval") {}
    CATCH_SECTION ("one minute boundary in less than a minute")
    {
        previous_seconds = 50;
        current_seconds = 70;
    }
    CATCH_SECTION ("an exact number of elapsed minutes")
    {
        previous_seconds = 10;
        current_seconds = 190;
    }
    CATCH_SECTION ("a backwards clock spanning a minute")
    {
        current_seconds = 70;
    }

    std::tm date {};
    date.tm_year = 124;
    date.tm_mon = 0;
    date.tm_mday = 2;
    date.tm_hour = 12;
    date.tm_isdst = -1;
    Time start = helpers::from_time_t (std::mktime (&date));
    auto const prefix = directory.path + LOG4CPLUS_TEXT ("/");
    auto const current = directory.file (LOG4CPLUS_TEXT ("current.log"));
    TestAppender appender (current,
        prefix + LOG4CPLUS_TEXT ("archive-%d{yyyy-MM-dd_HH-mm}.log"),
        2, false, true, false, false);
    auto const archive_name = [&] (int minute)
    {
        return prefix + helpers::getFormattedTime (
            LOG4CPLUS_TEXT ("archive-%Y-%m-%d_%H-%M.log"),
            start + std::chrono::minutes {minute}, false);
    };
    int const first_minute = previous_seconds / 60 - 2;
    int const last_minute = (std::max) (previous_seconds, current_seconds) / 60 + 1;
    for (int i = first_minute; i <= last_minute; ++i)
    {
        auto const name = archive_name (i);
        directory.files.push_back (name);
        tofstream archive (LOG4CPLUS_FSTREAM_PREFERED_FILE_NAME (name),
            std::ios_base::out | std::ios_base::trunc);
        archive << "archive " << i;
        CATCH_REQUIRE (archive.good ());
    }
    auto const unrelated = directory.file (LOG4CPLUS_TEXT ("unrelated.log"));
    {
        tofstream file (LOG4CPLUS_FSTREAM_PREFERED_FILE_NAME (unrelated),
            std::ios_base::out | std::ios_base::trunc);
        file << "unrelated";
        CATCH_REQUIRE (file.good ());
    }

    appender.lastHeartBeat = start + std::chrono::seconds {previous_seconds};
    Time now = start + std::chrono::seconds {current_seconds};
    for (int pass = 0; pass < 2; ++pass)
    {
        appender.clean (now);
        CATCH_CHECK (appender.lastHeartBeat == now);
        for (int i = first_minute; i <= last_minute; ++i)
        {
            CATCH_CAPTURE (i, pass, previous_seconds, current_seconds);
            bool const retained = current_seconds < previous_seconds
                || i >= current_seconds / 60 - 2;
            CATCH_CHECK (exists (archive_name (i)) == retained);
        }
    }
    CATCH_CHECK (exists (unrelated));
    CATCH_CHECK (exists (current));
}
#endif

} // namespace log4cplus

#if defined (LOG4CPLUS_WITH_UNIT_TESTS) \
    && defined (_WIN32)


namespace {

/** @brief Removes a test file if it exists. */
void erase (wchar_t const * path) {
    DeleteFileW (path);
}

/** @brief Reads a test file as uninterpreted bytes through Win32. */
std::vector<char> raw_read (wchar_t const * path) {
    std::vector<char> result;
    HANDLE const h =
        CreateFileW (path, GENERIC_READ,
                     FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                     nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        return result;
    }
    LARGE_INTEGER size = {};
    if (GetFileSizeEx (h, &size)) {
        result.resize (static_cast<std::size_t> (size.QuadPart));
        DWORD got = 0;
        if (!result.empty ()
            && !ReadFile (h, &result[0], static_cast<DWORD> (result.size ()),
                          &got, nullptr)) {
            result.clear ();
        } else {
            result.resize (got);
        }
    }
    CloseHandle (h);
    return result;
}

/** @brief Replaces a test file with the supplied uninterpreted bytes. */
bool raw_write (wchar_t const * path, char const * data, DWORD size) {
    HANDLE const h =
        CreateFileW (path, GENERIC_WRITE,
                     FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                     nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        return false;
    }
    DWORD written = 0;
    bool const ok =
        WriteFile (h, data, size, &written, nullptr) && written == size;
    CloseHandle (h);
    return ok;
}

CATCH_TEST_CASE ("An open file can be renamed", "[open][sharing]") {
    // Start with clean source and destination paths and open the source.
    wchar_t const * const old_name = L"rename-open-old.txt";
    wchar_t const * const new_name = L"rename-open-new.txt";
    erase (old_name);
    erase (new_name);
    log4cplus::helpers::win32_fstream fs (old_name, std::ios_base::out
                                                    | std::ios_base::trunc
                                                    | std::ios_base::binary);
    CATCH_INFO ("open rename source");
    CATCH_CHECK (fs.is_open ());

    // Write, rename the still-open file, and continue through the same handle.
    fs << "before" << std::flush;
    CATCH_INFO ("rename an open stream");
    CATCH_CHECK (MoveFileExW (old_name, new_name, MOVEFILE_REPLACE_EXISTING)
                 != 0);
    fs << "-after";
    fs.close ();

    // Verify that both writes followed the handle to the destination path.
    std::vector<char> bytes = raw_read (new_name);
    CATCH_INFO ("continued output follows renamed handle");
    CATCH_CHECK (std::string (bytes.begin (), bytes.end ()) == "before-after");
    erase (new_name);
}

CATCH_TEST_CASE ("Text mode converts UTF-8 and line endings",
                 "[unicode][text]") {
    // Write wide text containing LF and a non-ASCII BMP character.
    wchar_t const * const name = L"unicode-text.txt";
    erase (name);
    {
        log4cplus::helpers::win32_wfstream fs (name, std::ios_base::out
                                                     | std::ios_base::trunc);
        wchar_t text[] = {L'A', L'\n', static_cast<wchar_t> (0x5fc3), 0};
        fs.write (text, 3);
        fs.close ();
    }

    // Verify UTF-8 encoding and Windows text-mode CRLF expansion as raw bytes.
    std::vector<char> const bytes = raw_read (name);
    static char const expected[] = {'A',         '\r',        '\n',
                                    char (0xe5), char (0xbf), char (0x83)};
    CATCH_INFO ("wide text becomes UTF-8 with CRLF");
    CATCH_CHECK (bytes
                 == std::vector<char> (expected, expected + sizeof (expected)));

    // Read through the wide stream and verify UTF-8 and CRLF decoding.
    {
        log4cplus::helpers::win32_wfstream fs (name, std::ios_base::in);
        wchar_t text[4] = {};
        fs.read (text, 3);
        CATCH_INFO ("UTF-8 and CRLF decode to wide text");
        CATCH_CHECK (
            (text[0] == L'A' && text[1] == L'\n' && text[2] == 0x5fc3));
        fs.close ();
    }
    erase (name);
}

CATCH_TEST_CASE ("Text mode consumes a leading UTF-8 BOM", "[unicode][bom]") {
    // Create a raw UTF-8 file beginning with a BOM.
    wchar_t const * const name = L"bom.txt";
    char const bytes[] = {char (0xef), char (0xbb), char (0xbf), 'X'};
    CATCH_INFO ("create BOM input");
    CATCH_CHECK (raw_write (name, bytes, sizeof (bytes)));

    // Text mode consumes the leading BOM.
    log4cplus::helpers::win32_wfstream text (name, std::ios_base::in);
    CATCH_INFO ("text mode consumes initial BOM");
    CATCH_CHECK (text.get () == L'X');
    text.close ();

    // Binary mode exposes U+FEFF as ordinary decoded content.
    log4cplus::helpers::win32_wfstream binary (name, std::ios_base::in
                                                     | std::ios_base::binary);
    CATCH_INFO ("binary mode exposes BOM as content");
    CATCH_CHECK (binary.get () == static_cast<wchar_t> (0xfeff));
    binary.close ();
    erase (name);
}

CATCH_TEST_CASE ("Narrow conversion uses the code page replacement",
                 "[unicode][conversion]") {
    // Store a UTF-8 character that the C/ASCII code page cannot represent.
    wchar_t const * const name = L"replacement.txt";
    char const euro[] = {char (0xe2), char (0x82), char (0xac)};
    CATCH_INFO ("create replacement input");
    CATCH_CHECK (raw_write (name, euro, sizeof (euro)));

    // Decode through a narrow stream using replacement mode.
    log4cplus::helpers::win32_open_options opts;
    opts.conversion_errors = log4cplus::helpers::conversion_error_policy::replace;
    log4cplus::helpers::win32_fstream fs;
    fs.open (name, std::ios_base::in | std::ios_base::binary, opts);
    char actual = 0;
    fs.get (actual);

    // Verify that Windows selected the code page's configured default byte.
    CPINFO info = {};
    CATCH_INFO ("query ASCII default character");
    CATCH_CHECK (
        GetCPInfo (log4cplus::helpers::detail::windows_us_ascii_code_page, &info)
        != 0);
    CATCH_INFO ("replacement uses code-page configured default");
    CATCH_CHECK (static_cast<unsigned char> (actual) == info.DefaultChar[0]);
    fs.close ();
    erase (name);
}

CATCH_TEST_CASE ("Malformed UTF-8 obeys the conversion policy",
                 "[unicode][conversion]") {
    // Define representative malformed UTF-8 byte sequences followed by text.
    wchar_t const * const name = L"malformed.txt";
    char const invalid_lead[] = {char (0xff), 'x'};
    char const invalid_continuation[] = {char (0xe2), '(', char (0xa1), 'x'};
    char const overlong[] = {char (0xc0), char (0xaf), 'x'};
    char const surrogate[] = {char (0xed), char (0xa0), char (0x80), 'x'};
    char const too_large[] = {char (0xf4), char (0x90), char (0x80),
                              char (0x80), 'x'};
    char const truncated[] = {char (0xe2), char (0x82), 'x'};

    struct malformed_vector {
        char const * bytes;
        DWORD size;
        char const * description;
    };

    malformed_vector const vectors[] = {
        {invalid_lead, sizeof (invalid_lead), "invalid UTF-8 lead byte"},
        {invalid_continuation, sizeof (invalid_continuation),
         "invalid UTF-8 continuation byte"},
        {overlong, sizeof (overlong), "overlong UTF-8 encoding"},
        {surrogate, sizeof (surrogate), "UTF-8 encoded surrogate"},
        {too_large, sizeof (too_large), "UTF-8 value above U+10FFFF"},
        {truncated, sizeof (truncated), "truncated UTF-8 sequence"},
    };

    // Exercise both conversion policies for every malformed sequence.
    for (std::size_t i = 0; i != sizeof (vectors) / sizeof (vectors[0]); ++i) {
        malformed_vector const & vector = vectors[i];

        // Write bytes directly so no encoder can sanitize the malformed input.
        CATCH_INFO (vector.description);
        CATCH_CHECK (raw_write (name, vector.bytes, vector.size));

        // Replacement mode emits U+FFFD and resumes at later valid input.
        log4cplus::helpers::win32_open_options replace;
        replace.conversion_errors =
            log4cplus::helpers::conversion_error_policy::replace;
        log4cplus::helpers::win32_wfstream forgiving;
        forgiving.open (name, std::ios_base::in | std::ios_base::binary,
                        replace);
        std::wstring decoded;
        wchar_t character = 0;
        while (forgiving.get (character)) {
            decoded.push_back (character);
        }
        CATCH_INFO ("replacement mode emits U+FFFD for malformed UTF-8");
        CATCH_CHECK (std::find (decoded.begin (), decoded.end (),
                                static_cast<wchar_t> (0xfffd))
                     != decoded.end ());
        CATCH_INFO ("replacement mode continues after malformed UTF-8");
        CATCH_CHECK (
            (!decoded.empty () && decoded[decoded.size () - 1] == L'x'));
        forgiving.close ();

        // Strict mode stops immediately and preserves the conversion error.
        log4cplus::helpers::win32_wfstream strict (
            name, std::ios_base::in | std::ios_base::binary);
        CATCH_INFO ("strict mode rejects malformed UTF-8");
        CATCH_CHECK (strict.get () == std::char_traits<wchar_t>::eof ());
        CATCH_INFO ("strict malformed input retains diagnostic");
        CATCH_CHECK (
            strict.last_error ()
            == std::make_error_code (std::errc::illegal_byte_sequence));
        strict.close ();
    }
    erase (name);
}

CATCH_TEST_CASE ("Malformed UTF-16 output obeys the conversion policy",
                 "[unicode][conversion]") {
    // Define malformed UTF-16 sequences and their replacement-mode UTF-8 bytes.
    wchar_t const * const name = L"malformed-wide.txt";
    wchar_t const lone_low[] = {static_cast<wchar_t> (0xdc00), L'x'};
    wchar_t const high_then_normal[] = {static_cast<wchar_t> (0xd800), L'x'};
    wchar_t const two_highs[] = {static_cast<wchar_t> (0xd800),
                                 static_cast<wchar_t> (0xd801), L'x'};
    wchar_t const trailing_high[] = {static_cast<wchar_t> (0xd800)};

    char const replacement_then_x[] = {char (0xef), char (0xbf), char (0xbd),
                                       'x'};
    char const two_replacements_then_x[] = {
        char (0xef), char (0xbf), char (0xbd), char (0xef),
        char (0xbf), char (0xbd), 'x'};
    char const replacement_only[] = {char (0xef), char (0xbf), char (0xbd)};

    struct malformed_vector {
        wchar_t const * input;
        std::streamsize input_size;
        char const * expected;
        std::size_t expected_size;
        char const * description;
    };

    malformed_vector const vectors[] = {
        {lone_low, sizeof (lone_low) / sizeof (lone_low[0]), replacement_then_x,
         sizeof (replacement_then_x), "lone low surrogate"},
        {high_then_normal,
         sizeof (high_then_normal) / sizeof (high_then_normal[0]),
         replacement_then_x, sizeof (replacement_then_x),
         "high surrogate followed by a normal character"},
        {two_highs, sizeof (two_highs) / sizeof (two_highs[0]),
         two_replacements_then_x, sizeof (two_replacements_then_x),
         "two consecutive high surrogates"},
        {trailing_high, sizeof (trailing_high) / sizeof (trailing_high[0]),
         replacement_only, sizeof (replacement_only),
         "trailing high surrogate"},
    };

    // Exercise strict and replacement output policies for every sequence.
    for (std::size_t i = 0; i != sizeof (vectors) / sizeof (vectors[0]); ++i) {
        malformed_vector const & vector = vectors[i];

        // Strict mode rejects malformed UTF-16 and records the diagnostic.
        log4cplus::helpers::win32_wfstream strict (
            name,
            std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
        strict.write (vector.input, vector.input_size);
        strict.close ();
        CATCH_INFO (vector.description);
        CATCH_CHECK (strict.fail ());
        CATCH_INFO ("strict malformed UTF-16 output retains diagnostic");
        CATCH_CHECK (
            strict.last_error ()
            == std::make_error_code (std::errc::illegal_byte_sequence));

        // Replacement mode writes U+FFFD and continues when input remains.
        log4cplus::helpers::win32_open_options replace;
        replace.conversion_errors =
            log4cplus::helpers::conversion_error_policy::replace;
        log4cplus::helpers::win32_wfstream forgiving;
        forgiving.open (name,
                        std::ios_base::out | std::ios_base::trunc
                            | std::ios_base::binary,
                        replace);
        forgiving.write (vector.input, vector.input_size);
        forgiving.close ();
        CATCH_INFO ("replacement mode accepts malformed UTF-16 output");
        CATCH_CHECK (!forgiving.fail ());

        // Verify the exact replacement bytes written to the file.
        std::vector<char> const actual = raw_read (name);
        std::vector<char> const expected (
            vector.expected, vector.expected + vector.expected_size);
        CATCH_INFO ("malformed UTF-16 output has expected replacement bytes");
        CATCH_CHECK (actual == expected);
    }

    erase (name);
}

CATCH_TEST_CASE ("Sharing and append modes preserve Win32 semantics",
                 "[open][sharing][append]") {
    // Remove delete sharing and verify that an open handle prevents rename.
    wchar_t const * const old_name = L"sharing-old.txt";
    wchar_t const * const new_name = L"sharing-new.txt";
    erase (old_name);
    erase (new_name);
    log4cplus::helpers::win32_open_options narrow;
    narrow.share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    log4cplus::helpers::win32_fstream locked;
    locked.open (old_name, std::ios_base::out | std::ios_base::trunc, narrow);
    CATCH_INFO ("share option can deny rename");
    CATCH_CHECK (MoveFileExW (old_name, new_name, MOVEFILE_REPLACE_EXISTING)
                 == 0);
    locked.close ();
    erase (old_name);
    erase (new_name);

    // Seed the append target to verify that app mode does not truncate it.
    wchar_t const * const append_name = L"append.txt";
    char const prefix[] = {'p', 'r', 'e', 'f', 'i', 'x', '-'};
    erase (append_name);
    CATCH_INFO ("create existing append target");
    CATCH_CHECK (raw_write (append_name, prefix, sizeof (prefix)));

    // Open two append streams and interleave writes to the shared file.
    log4cplus::helpers::win32_fstream a (append_name, std::ios_base::out
                                                      | std::ios_base::app
                                                      | std::ios_base::binary);
    log4cplus::helpers::win32_fstream b (append_name, std::ios_base::out
                                                      | std::ios_base::app
                                                      | std::ios_base::binary);
    a << 'A' << std::flush;
    b << 'B' << std::flush;

    // A seek changes the logical cursor but app mode must still write at EOF.
    a.seekp (0, std::ios_base::beg);
    CATCH_INFO ("seek an app-mode stream");
    CATCH_CHECK (!a.fail ());
    a << 'C' << std::flush;
    a.close ();
    b.close ();

    // Verify preservation and that every physical write targeted current EOF.
    std::vector<char> bytes = raw_read (append_name);
    CATCH_INFO ("each app-mode write targets current EOF");
    CATCH_CHECK (std::string (bytes.begin (), bytes.end ()) == "prefix-ABC");
    erase (append_name);
}

CATCH_TEST_CASE ("Byte positions can be saved and restored", "[seek]") {
    // Create a bidirectional binary stream with known contents.
    wchar_t const * const name = L"seek.txt";
    erase (name);
    log4cplus::helpers::win32_fstream fs (
        name, std::ios_base::in | std::ios_base::out | std::ios_base::trunc
                  | std::ios_base::binary);
    fs << "abc" << std::flush;

    // Verify beginning seeks and restoration of a position returned by tellg.
    fs.seekg (0, std::ios_base::beg);
    CATCH_INFO ("seek to beginning");
    CATCH_CHECK (fs.get () == 'a');
    std::streampos const saved = fs.tellg ();
    CATCH_INFO ("read after tell");
    CATCH_CHECK (fs.get () == 'b');
    fs.seekg (saved);
    CATCH_INFO ("seek to saved position");
    CATCH_CHECK (fs.get () == 'b');

    // Treat nonzero current-relative offsets as external byte counts.
    fs.clear ();
    fs.seekg (1, std::ios_base::cur);
    CATCH_INFO ("accept a current-relative byte seek");
    CATCH_CHECK ((!fs.fail () && fs.peek () == std::char_traits<char>::eof ()));
    fs.close ();
    erase (name);
}

CATCH_TEST_CASE ("Seeking resynchronizes at UTF-8 boundaries",
                 "[seek][unicode]") {
    // Create adjacent two-, three-, and four-byte UTF-8 sequences.
    wchar_t const * const name = L"multibyte-seek.txt";
    char const bytes[] = {'A',         char (0xc2), char (0xa2), char (0xe3),
                          char (0x81), char (0x8a), char (0xf0), char (0x93),
                          char (0x80), char (0x84), 'B'};
    erase (name);
    CATCH_INFO ("create multibyte seek input");
    CATCH_CHECK (raw_write (name, bytes, sizeof (bytes)));

    log4cplus::helpers::win32_wfstream fs (name, std::ios_base::in
                                                 | std::ios_base::binary);
    CATCH_INFO ("open multibyte seek input");
    CATCH_CHECK (fs.is_open ());

    // Positions returned by tellg remain exact and restorable.
    CATCH_INFO ("read ASCII before multibyte character");
    CATCH_CHECK (fs.get () == L'A');
    std::wstreampos const before = fs.tellg ();
    CATCH_INFO ("read two-byte character");
    CATCH_CHECK (fs.get () == L'\u00a2');
    std::wstreampos const after = fs.tellg ();
    fs.seekg (before);
    CATCH_INFO ("restore position before two-byte character");
    CATCH_CHECK (fs.get () == L'\u00a2');
    fs.seekg (after);
    CATCH_INFO ("restore position after two-byte character");
    CATCH_CHECK (fs.get () == L'\u304a');

    // Every continuation-byte target advances to the next leading byte.
    struct resynchronization_vector {
        std::streamoff requested;
        std::streamoff adjusted;
        wchar_t expected;
        bool expected_is_high_surrogate;
        char const * description;
    };

    resynchronization_vector const vectors[] = {
        {2, 3, L'\u304a', false, "two-byte continuation"},
        {4, 6, 0, true, "first three-byte continuation"},
        {5, 6, 0, true, "second three-byte continuation"},
        {7, 10, L'B', false, "first four-byte continuation"},
        {8, 10, L'B', false, "second four-byte continuation"},
        {9, 10, L'B', false, "third four-byte continuation"},
    };
    for (std::size_t i = 0; i != sizeof (vectors) / sizeof (vectors[0]); ++i) {
        resynchronization_vector const & vector = vectors[i];
        fs.clear ();
        fs.seekg (std::wstreampos (vector.requested));
        bool const position_matches =
            !fs.fail ()
            && static_cast<std::streamoff> (fs.tellg ()) == vector.adjusted;
        wchar_t const actual = fs.get ();
        bool const character_matches =
            vector.expected_is_high_surrogate
                ? log4cplus::helpers::detail::is_high_surrogate (actual)
                : actual == vector.expected;
        CATCH_INFO (vector.description);
        CATCH_CHECK ((position_matches && character_matches));
    }

    // Current-relative seeking is undefined between UTF-16 surrogate units.
    fs.clear ();
    fs.seekg (6, std::ios_base::beg);
    CATCH_INFO ("extract the first code unit of a supplementary scalar");
    CATCH_CHECK (log4cplus::helpers::detail::is_high_surrogate (fs.get ()));
    fs.seekg (1, std::ios_base::cur);
    CATCH_INFO ("reject current seek inside decoded scalar output");
    CATCH_CHECK (fs.fail ());

    // Beginning-, current-, and end-relative byte offsets all resynchronize.
    fs.clear ();
    fs.seekg (4, std::ios_base::beg);
    CATCH_INFO ("resynchronize a beginning-relative byte offset");
    CATCH_CHECK (
        (!fs.fail () && static_cast<std::streamoff> (fs.tellg ()) == 6));
    fs.seekg (0, std::ios_base::beg);
    CATCH_INFO ("restart before a current-relative seek");
    CATCH_CHECK (fs.get () == L'A');
    fs.seekg (1, std::ios_base::cur);
    CATCH_INFO ("resynchronize a current-relative byte offset");
    CATCH_CHECK (
        (!fs.fail () && static_cast<std::streamoff> (fs.tellg ()) == 3));
    fs.seekg (-2, std::ios_base::cur);
    CATCH_INFO ("apply a negative current-relative byte offset");
    CATCH_CHECK ((!fs.fail () && static_cast<std::streamoff> (fs.tellg ()) == 1
                  && fs.get () == L'\u00a2'));
    fs.seekg (-4, std::ios_base::end);
    CATCH_INFO ("resynchronize an end-relative byte offset");
    CATCH_CHECK ((!fs.fail () && static_cast<std::streamoff> (fs.tellg ()) == 10
                  && fs.get () == L'B'));

    // Positions beyond EOF remain unchanged and read as EOF.
    fs.clear ();
    fs.seekg (2, std::ios_base::end);
    CATCH_INFO ("preserve a byte position beyond EOF");
    CATCH_CHECK ((!fs.fail () && static_cast<std::streamoff> (fs.tellg ()) == 13
                  && fs.peek () == std::char_traits<wchar_t>::eof ()));

    // Reject targets before byte zero and targets outside streamoff range.
    fs.clear ();
    fs.seekg (-1, std::ios_base::beg);
    CATCH_INFO ("reject a negative absolute byte position");
    CATCH_CHECK (fs.fail ());
    fs.clear ();
    fs.seekg ((std::numeric_limits<std::streamoff>::max) (),
              std::ios_base::end);
    CATCH_INFO ("reject seek byte arithmetic overflow");
    CATCH_CHECK (fs.fail ());

    fs.close ();
    erase (name);
}

CATCH_TEST_CASE ("Seeking preserves malformed input diagnostics",
                 "[seek][unicode][conversion]") {
    // A malformed non-continuation lead remains a syntactic seek boundary.
    wchar_t const * const name = L"malformed-seek.txt";
    char const bytes[] = {'A', char (0xff), 'B'};
    erase (name);
    CATCH_INFO ("create malformed seek input");
    CATCH_CHECK (raw_write (name, bytes, sizeof (bytes)));

    log4cplus::helpers::win32_wfstream strict (name, std::ios_base::in
                                                     | std::ios_base::binary);
    strict.seekg (1, std::ios_base::beg);
    CATCH_INFO ("seek does not skip a malformed leading byte");
    CATCH_CHECK (static_cast<std::streamoff> (strict.tellg ()) == 1);
    CATCH_INFO ("strict conversion still reports malformed seek input");
    CATCH_CHECK (
        (strict.get () == std::char_traits<wchar_t>::eof ()
         && strict.last_error ()
                == std::make_error_code (std::errc::illegal_byte_sequence)));
    strict.close ();

    // Replacement mode observes the same byte and then continues normally.
    log4cplus::helpers::win32_open_options replace;
    replace.conversion_errors =
        log4cplus::helpers::conversion_error_policy::replace;
    log4cplus::helpers::win32_wfstream forgiving;
    forgiving.open (name, std::ios_base::in | std::ios_base::binary, replace);
    forgiving.seekg (1, std::ios_base::beg);
    CATCH_INFO ("replacement conversion remains active after a seek");
    CATCH_CHECK ((forgiving.get () == static_cast<wchar_t> (0xfffd)
                  && forgiving.get () == L'B'));
    forgiving.close ();

    // A malformed continuation run is traversed without being decoded.
    char const continuations[] = {'A',         char (0x80), char (0x81),
                                  char (0x82), char (0x83), char (0x84),
                                  'B'};
    CATCH_INFO ("create malformed continuation seek input");
    CATCH_CHECK (raw_write (name, continuations, sizeof (continuations)));
    log4cplus::helpers::win32_wfstream resynchronized (
        name, std::ios_base::in | std::ios_base::binary);
    resynchronized.seekg (1, std::ios_base::beg);
    CATCH_INFO ("seek traverses a long malformed continuation run");
    CATCH_CHECK ((!resynchronized.fail ()
                  && static_cast<std::streamoff> (resynchronized.tellg ()) == 6
                  && resynchronized.get () == L'B'));
    resynchronized.close ();
    erase (name);
}

CATCH_TEST_CASE ("Output seeking uses adjusted byte positions",
                 "[seek][output][append]") {
    // Build UTF-8 through an output-only handle that also has read access.
    wchar_t const * const name = L"output-seek.txt";
    erase (name);
    log4cplus::helpers::win32_wfstream output (name, std::ios_base::out
                                                     | std::ios_base::trunc
                                                     | std::ios_base::binary);
    wchar_t const initial[] = {L'A', L'\u304a', L'B'};
    output.write (initial, 3);
    output.flush ();
    std::wstreampos const original_end = output.tellp ();

    // Verify the output-only native handle can inspect seek targets.
    char first = 0;
    DWORD got = 0;
    OVERLAPPED read_at_zero = {};
    CATCH_INFO ("output-only handle includes read access");
    CATCH_CHECK (
        (ReadFile (output.native_handle (), &first, 1, &got, &read_at_zero)
         && got == 1 && first == 'A'));

    // An absolute output target inside U+304A advances to its following byte.
    output.seekp (std::wstreampos (2));
    CATCH_INFO ("resynchronize an output-only byte position");
    CATCH_CHECK ((!output.fail ()
                  && static_cast<std::streamoff> (output.tellp ()) == 4));
    output.put (L'X');

    // Positions returned by tellp remain exactly restorable without history.
    output.seekp (original_end);
    CATCH_INFO ("restore an output position returned by tellp");
    CATCH_CHECK ((!output.fail ()
                  && static_cast<std::streamoff> (output.tellp ()) == 5));
    output.put (L'Y');

    // Seeking beyond EOF permits a later write to create a zero-filled gap.
    output.seekp (7, std::ios_base::beg);
    CATCH_INFO ("preserve output position beyond EOF");
    CATCH_CHECK ((!output.fail ()
                  && static_cast<std::streamoff> (output.tellp ()) == 7));
    output.put (L'Z');
    output.close ();
    char const expected[] = {'A', char (0xe3), char (0x81), char (0x8a),
                             'X', 'Y',         0,           'Z'};
    CATCH_INFO ("output seeking overwrites at adjusted and sparse positions");
    CATCH_CHECK (raw_read (name)
                 == std::vector<char> (expected, expected + sizeof (expected)));

    // App mode still writes at EOF after its logical target is adjusted.
    log4cplus::helpers::win32_wfstream append (
        name, std::ios_base::out | std::ios_base::app | std::ios_base::binary);
    append.seekp (std::wstreampos (2));
    CATCH_INFO ("resynchronize an app-mode logical position");
    CATCH_CHECK ((!append.fail ()
                  && static_cast<std::streamoff> (append.tellp ()) == 4));
    append.put (L'C');
    append.close ();
    std::vector<char> const appended = raw_read (name);
    CATCH_INFO ("app mode ignores adjusted logical position for writes");
    CATCH_CHECK ((appended.size () == sizeof (expected) + 1
                  && appended[appended.size () - 1] == 'C'));
    erase (name);
}

CATCH_TEST_CASE ("External byte buffers can be configured", "[buffer]") {
    wchar_t const * const name = L"configured-buffer.txt";
    erase (name);

    // Reject storage that cannot hold one complete UTF-8 sequence.
    log4cplus::helpers::win32_fstream invalid;
    char too_small[3] = {};
    CATCH_INFO ("reject a three-byte external buffer");
    CATCH_CHECK (invalid.rdbuf ()->pubsetbuf (too_small, 3) == nullptr);
    CATCH_INFO ("small external buffer reports invalid argument");
    CATCH_CHECK (invalid.last_error ()
                 == std::make_error_code (std::errc::invalid_argument));

    // Reject inconsistent null/count pairs without changing configuration.
    char adequate[4] = {};
    CATCH_INFO ("reject null storage with a nonzero size");
    CATCH_CHECK (invalid.rdbuf ()->pubsetbuf (nullptr, 1) == nullptr);
    CATCH_INFO ("reject non-null storage with a zero size");
    CATCH_CHECK (invalid.rdbuf ()->pubsetbuf (adequate, 0) == nullptr);

    // Reject element counts whose byte capacity would overflow size_t.
    log4cplus::helpers::win32_u32fstream overflow;
    char32_t overflow_storage[1] = {};
    CATCH_INFO ("reject overflowing external buffer size");
    CATCH_CHECK (
        overflow.rdbuf ()->pubsetbuf (
            overflow_storage, (std::numeric_limits<std::streamsize>::max) ())
        == nullptr);

    // Use exactly four caller-owned bytes for supplementary UTF-8 output.
    log4cplus::helpers::win32_wfstream output;
    wchar_t external[2] = {};
    CATCH_INFO ("accept a four-byte external buffer");
    CATCH_CHECK (output.rdbuf ()->pubsetbuf (external, 2) == output.rdbuf ());
    output.open (name, std::ios_base::out | std::ios_base::trunc
                           | std::ios_base::binary);
    CATCH_INFO ("open output with an external buffer");
    CATCH_CHECK (output.is_open ());

    // Buffer replacement is forbidden after the file has been opened.
    wchar_t replacement[2] = {};
    CATCH_INFO ("reject buffer replacement after open");
    CATCH_CHECK (output.rdbuf ()->pubsetbuf (replacement, 2) == nullptr);
    wchar_t const hieroglyph[] = L"\U00013004";
    output.write (hieroglyph,
                  static_cast<std::streamsize> (
                      sizeof (hieroglyph) / sizeof (hieroglyph[0]) - 1));
    output.close ();
    char const expected[] = {char (0xf0), char (0x93), char (0x80),
                             char (0x84)};
    CATCH_INFO ("external output buffer preserves supplementary UTF-8");
    CATCH_CHECK (raw_read (name)
                 == std::vector<char> (expected, expected + sizeof (expected)));

    // A null zero-sized request selects the four-byte internal staging mode.
    log4cplus::helpers::win32_wfstream minimal;
    CATCH_INFO ("accept minimally buffered mode");
    CATCH_CHECK (minimal.rdbuf ()->pubsetbuf (nullptr, 0) == minimal.rdbuf ());
    minimal.open (name, std::ios_base::in | std::ios_base::binary);
    wchar_t decoded[2] = {};
    minimal.read (decoded, 2);
    CATCH_INFO ("minimal input buffer decodes supplementary UTF-8");
    CATCH_CHECK ((decoded[0] == hieroglyph[0] && decoded[1] == hieroglyph[1]));
    minimal.close ();

    // The selected minimal buffer persists across close and reopen.
    minimal.open (name, std::ios_base::out | std::ios_base::trunc
                            | std::ios_base::binary);
    minimal.put (L'Z');
    minimal.close ();
    CATCH_INFO ("minimal buffer persists for later output");
    CATCH_CHECK (raw_read (name) == std::vector<char> (1, 'Z'));
    erase (name);
}

CATCH_TEST_CASE ("Buffered input spans encoding boundaries",
                 "[buffer][unicode][text]") {
    // Arrange BOM, CRLF, BMP, and supplementary sequences across refills.
    wchar_t const * const name = L"buffer-boundaries.txt";
    char const bytes[] = {char (0xef), char (0xbb), char (0xbf), 'A',
                          '\r',        '\n',        char (0xe3), char (0x81),
                          char (0x8a), char (0xf0), char (0x93), char (0x80),
                          char (0x84), 'Z'};
    erase (name);
    CATCH_INFO ("create buffered boundary input");
    CATCH_CHECK (raw_write (name, bytes, sizeof (bytes)));

    // Decode through an external buffer whose raw capacity is four bytes.
    log4cplus::helpers::win32_wfstream fs;
    wchar_t external[2] = {};
    CATCH_INFO ("configure four-byte input buffer");
    CATCH_CHECK (fs.rdbuf ()->pubsetbuf (external, 2) == fs.rdbuf ());
    fs.open (name, std::ios_base::in);
    wchar_t actual[6] = {};
    fs.read (actual, 6);
    wchar_t const expected[] = {L'A',
                                L'\n',
                                L'\u304a',
                                static_cast<wchar_t> (0xd80c),
                                static_cast<wchar_t> (0xdc04),
                                L'Z'};
    CATCH_INFO ("bulk input handles UTF-8 and text boundaries");
    CATCH_CHECK (
        (fs.gcount () == 6 && std::equal (actual, actual + 6, expected)));
    fs.close ();
    erase (name);
}

CATCH_TEST_CASE ("Buffered streams preserve positions across directions",
                 "[buffer][direction]") {
    wchar_t const * const name = L"buffer-direction.txt";
    char const original[] = {'A', 'B', 'C', 'D', 'E'};
    erase (name);
    CATCH_INFO ("create direction-change input");
    CATCH_CHECK (raw_write (name, original, sizeof (original)));

    // Read ahead through a four-byte cache but consume only one character.
    log4cplus::helpers::win32_fstream fs;
    char external[4] = {};
    CATCH_INFO ("configure direction-change buffer");
    CATCH_CHECK (fs.rdbuf ()->pubsetbuf (external, 4) == fs.rdbuf ());
    fs.open (name,
             std::ios_base::in | std::ios_base::out | std::ios_base::binary);
    CATCH_INFO ("consume one buffered input character");
    CATCH_CHECK (fs.get () == 'A');
    CATCH_INFO ("bulk read fills the caller-owned buffer");
    CATCH_CHECK (std::string (external, external + 4) == "ABCD");

    // Switching to output must use the consumed position, not read-ahead EOF.
    fs.put ('X');
    fs.flush ();
    fs.seekg (0, std::ios_base::beg);
    char actual[5] = {};
    fs.read (actual, 5);
    CATCH_INFO ("read-ahead does not move the logical write position");
    CATCH_CHECK (std::string (actual, actual + 5) == "AXCDE");

    // A current-position query reports consumed bytes, not prefetched bytes.
    fs.clear ();
    fs.seekg (0, std::ios_base::beg);
    CATCH_INFO ("restart buffered position query");
    CATCH_CHECK (fs.get () == 'A');
    CATCH_INFO ("tellg ignores prefetched input bytes");
    CATCH_CHECK (static_cast<std::streamoff> (fs.tellg ()) == 1);
    fs.close ();
    erase (name);
}

CATCH_TEST_CASE ("Putback preserves one extracted code unit",
                 "[buffer][putback]") {
    // Include single-unit, BMP, and supplementary wide characters.
    wchar_t const * const name = L"putback.txt";
    char const bytes[] = {'A',         char (0xe3), char (0x81), char (0x8a),
                          'B',         char (0xf0), char (0x93), char (0x80),
                          char (0x84), 'C'};
    erase (name);
    CATCH_INFO ("create putback input");
    CATCH_CHECK (raw_write (name, bytes, sizeof (bytes)));

    log4cplus::helpers::win32_wfstream fs;
    wchar_t external[2] = {};
    CATCH_INFO ("configure putback input buffer");
    CATCH_CHECK (fs.rdbuf ()->pubsetbuf (external, 2) == fs.rdbuf ());
    fs.open (name, std::ios_base::in | std::ios_base::binary);

    // Immediate unget restores the last extracted code unit exactly once.
    CATCH_INFO ("extract character for immediate putback");
    CATCH_CHECK (fs.get () == L'A');
    fs.unget ();
    CATCH_INFO ("put back the immediately preceding character");
    CATCH_CHECK (!fs.fail ());
    fs.unget ();
    CATCH_INFO ("reject putback before the available history");
    CATCH_CHECK (fs.fail ());
    fs.clear ();
    CATCH_INFO ("re-extract an immediately put-back character");
    CATCH_CHECK (fs.get () == L'A');
    fs.putback (L'A');
    CATCH_INFO ("same-character putback restores the previous character");
    CATCH_CHECK ((!fs.fail () && fs.get () == L'A'));

    // Lookahead must not discard the previously extracted character.
    CATCH_INFO ("peek after an extracted character");
    CATCH_CHECK (fs.peek () == L'\u304a');
    fs.unget ();
    CATCH_INFO ("putback survives a subsequent underflow caused by peek");
    CATCH_CHECK ((!fs.fail () && fs.get () == L'A' && fs.get () == L'\u304a'));

    // Replacing the previous character with a different value is unsupported.
    fs.putback (L'X');
    CATCH_INFO ("reject mismatched putback character");
    CATCH_CHECK (fs.fail ());
    fs.clear ();
    CATCH_INFO ("continue after rejected mismatched putback");
    CATCH_CHECK (fs.get () == L'B');

    // Preserve the last UTF-16 code unit of a supplementary scalar.
    wchar_t const high = fs.get ();
    wchar_t const low = fs.get ();
    CATCH_INFO ("extract a supplementary UTF-16 surrogate pair");
    CATCH_CHECK ((log4cplus::helpers::detail::is_high_surrogate (high)
                  && log4cplus::helpers::detail::is_low_surrogate (low)));
    CATCH_INFO ("peek beyond a supplementary character");
    CATCH_CHECK (fs.peek () == L'C');
    fs.unget ();
    CATCH_INFO ("reject tellg inside a decoded supplementary scalar");
    CATCH_CHECK (fs.tellg () == std::wstreampos (std::streamoff (-1)));
    fs.clear ();
    CATCH_INFO ("put back the last supplementary code unit after lookahead");
    CATCH_CHECK ((!fs.fail () && fs.get () == low && fs.get () == L'C'));

    // EOF probing retains the last extracted character for putback.
    CATCH_INFO ("probe EOF after the final character");
    CATCH_CHECK (fs.peek () == std::char_traits<wchar_t>::eof ());
    fs.clear ();
    fs.unget ();
    CATCH_INFO ("putback survives an EOF probe");
    CATCH_CHECK ((!fs.fail () && fs.get () == L'C'));
    fs.close ();
    erase (name);
}

/** @brief Verifies one UTF-8 file round trip for a stream character type. */
template <typename CharT>
void typed_round_trip (wchar_t const * name, CharT value) {
    erase (name);

    // Encode one character of the selected stream type as UTF-8.
    {
        log4cplus::helpers::basic_win32_fstream<CharT> fs (
            name,
            std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
        fs.write (&value, 1);
        fs.close ();
        CATCH_INFO ("typed UTF-8 output");
        CATCH_CHECK (!fs.fail ());
    }

    // Decode the file back into the same stream character type.
    {
        log4cplus::helpers::basic_win32_fstream<CharT> fs (
            name, std::ios_base::in | std::ios_base::binary);
        CharT actual = 0;
        fs.read (&actual, 1);
        CATCH_INFO ("typed UTF-8 round trip");
        CATCH_CHECK (actual == value);
        fs.close ();
    }
    erase (name);
}

/** @brief Verifies exact UTF-8 bytes and wide-character decoding. */
template <std::size_t InputSize, std::size_t ExpectedSize>
void wide_utf8_bytes_test (char const * file_name,
                           wchar_t const (&input)[InputSize],
                           char const (&expected)[ExpectedSize],
                           char const * description) {
    DeleteFileA (file_name);

    // Encode the escaped wide input through the custom stream.
    log4cplus::helpers::win32_wfstream output (
        file_name,
        std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
    CATCH_INFO ("open wide UTF-8 test output");
    CATCH_CHECK (output.is_open ());
    output.write (input, InputSize - 1);
    output.close ();
    CATCH_INFO ("write wide UTF-8 test output");
    CATCH_CHECK (!output.fail ());

    // Read with std::ifstream and compare the exact external UTF-8 bytes.
    std::ifstream bytes (file_name, std::ios_base::in | std::ios_base::binary);
    CATCH_INFO ("open UTF-8 output with std::ifstream");
    CATCH_CHECK (bytes.is_open ());
    std::string const actual ((std::istreambuf_iterator<char> (bytes)),
                              std::istreambuf_iterator<char> ());
    std::string const wanted (expected, ExpectedSize - 1);
    CATCH_INFO (description);
    CATCH_CHECK (actual == wanted);

    bytes.close ();

    // Decode through the custom wide stream and compare with the original.
    log4cplus::helpers::win32_wfstream decoded (
        file_name, std::ios_base::in | std::ios_base::binary);
    CATCH_INFO ("reopen UTF-8 output as a wide stream");
    CATCH_CHECK (decoded.is_open ());
    std::wstring round_trip (InputSize - 1, L'\0');
    decoded.read (&round_trip[0], InputSize - 1);
    std::wstring const original (input, InputSize - 1);
    CATCH_INFO ("UTF-8 output decodes to the original wide string");
    CATCH_CHECK (
        (decoded.gcount () == static_cast<std::streamsize> (InputSize - 1)
         && round_trip == original));
    decoded.close ();

    DeleteFileA (file_name);
}

CATCH_TEST_CASE ("Wide strings have exact UTF-8 representations",
                 "[unicode][round-trip]") {
    // Japanese ohayō written entirely with universal character escapes.
    wchar_t const ohayo[] = L"\u304a\u306f\u3088\u3046";
    char const ohayo_utf8[] =
        "\xe3\x81\x8a\xe3\x81\xaf\xe3\x82\x88\xe3\x81\x86";
    wide_utf8_bytes_test ("ohayo-utf8.txt", ohayo, ohayo_utf8,
                          "Japanese ohayo has expected UTF-8 bytes");

    // Icelandic “ábyrgð” with escapes for the non-ASCII characters.
    wchar_t const abyrgd[] = L"\u00e1byrg\u00f0";
    char const abyrgd_utf8[] = "\xc3\xa1"
                               "byrg"
                               "\xc3\xb0";
    wide_utf8_bytes_test ("abyrgd-utf8.txt", abyrgd, abyrgd_utf8,
                          "Icelandic abyrgd has expected UTF-8 bytes");

    // Egyptian hieroglyph U+13004 exercises a character outside the BMP.
    wchar_t const hieroglyph[] = L"\U00013004";
    char const hieroglyph_utf8[] = "\xf0\x93\x80\x84";
    wide_utf8_bytes_test ("hieroglyph-utf8.txt", hieroglyph, hieroglyph_utf8,
                          "out-of-BMP hieroglyph has expected UTF-8 bytes");
}

CATCH_TEST_CASE ("UTF-16 stream characters round trip through UTF-8",
                 "[unicode][round-trip]") {
    typed_round_trip<char16_t> (L"u16.txt", static_cast<char16_t> (0x5fc3));
}

CATCH_TEST_CASE ("UTF-32 stream characters round trip through UTF-8",
                 "[unicode][round-trip]") {
    typed_round_trip<char32_t> (L"u32.txt", static_cast<char32_t> (0x1f642));
}

#if defined(__cpp_char8_t)
CATCH_TEST_CASE ("UTF-8 stream characters round trip through UTF-8",
                 "[unicode][round-trip]") {
    typed_round_trip<char8_t> (L"u8.txt", static_cast<char8_t> ('x'));
}
#endif

} // namespace

#endif // _WIN32 && LOG4CPLUS_WITH_UNIT_TESTS
