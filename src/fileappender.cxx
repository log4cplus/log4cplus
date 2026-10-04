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
#include <memory>
#include <sstream>
#include <cstdio>
#include <stdexcept>
#include <cmath> // std::fmod
#include <filesystem>

// For _wrename() and _wremove() on Windows.
#include <stdio.h>
#include <cerrno>
#ifdef LOG4CPLUS_HAVE_ERRNO_H
#include <errno.h>
#endif

#if defined (LOG4CPLUS_WITH_UNIT_TESTS)
#include <catch_amalgamated.hpp>
#include <fstream>
#include <iterator>
#include <optional>
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
    if (useLockFile && ! lockFile)
    {
        if (createDirs)
            internal::make_dirs (lockFileName);

        try
        {
            lockFile = std::make_unique<helpers::LockFile> (lockFileName);
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

    out.open(std::filesystem::path (filename), mode);

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
    bool rollOnClose_, const tstring& datePattern_, FirstDayOfWeek firstDayOfWeek_)
    : FileAppender(filename_, std::ios_base::app, immediateFlush_, createDirs_)
    , firstDayOfWeek(firstDayOfWeek_)
    , maxBackupIndex(maxBackupIndex_)
    , rollOnClose(rollOnClose_)
    , datePattern(datePattern_)
{
    init(schedule_);
}



DailyRollingFileAppender::DailyRollingFileAppender(
    const Properties& properties)
    : FileAppender(properties, std::ios_base::app)
    , firstDayOfWeek(FirstDayOfWeek::MONDAY)
    , maxBackupIndex(10)
    , rollOnClose(true)
{
    DailyRollingFileSchedule theSchedule = DailyRollingFileSchedule::DAILY;
    tstring scheduleStr (helpers::toUpper (
        properties.getProperty (LOG4CPLUS_TEXT ("Schedule"))));

    if(scheduleStr == LOG4CPLUS_TEXT("MONTHLY"))
        theSchedule = DailyRollingFileSchedule::MONTHLY;
    else if(scheduleStr == LOG4CPLUS_TEXT("WEEKLY"))
        theSchedule = DailyRollingFileSchedule::WEEKLY;
    else if(scheduleStr == LOG4CPLUS_TEXT("DAILY"))
        theSchedule = DailyRollingFileSchedule::DAILY;
    else if(scheduleStr == LOG4CPLUS_TEXT("TWICE_DAILY"))
        theSchedule = DailyRollingFileSchedule::TWICE_DAILY;
    else if(scheduleStr == LOG4CPLUS_TEXT("HOURLY"))
        theSchedule = DailyRollingFileSchedule::HOURLY;
    else if(scheduleStr == LOG4CPLUS_TEXT("MINUTELY"))
        theSchedule = DailyRollingFileSchedule::MINUTELY;
    else {
        helpers::getLogLog().warn(
            LOG4CPLUS_TEXT("DailyRollingFileAppender::ctor()")
            LOG4CPLUS_TEXT("- \"Schedule\" not valid: ")
            + properties.getProperty(LOG4CPLUS_TEXT("Schedule")));
        theSchedule = DailyRollingFileSchedule::DAILY;
    }

    properties.getBool (rollOnClose, LOG4CPLUS_TEXT("RollOnClose"));
    properties.getString (datePattern, LOG4CPLUS_TEXT("DatePattern"));
    properties.getInt (maxBackupIndex, LOG4CPLUS_TEXT("MaxBackupIndex"));

    tstring firstDayOfWeekStr;
    if (properties.getString (firstDayOfWeekStr, LOG4CPLUS_TEXT("FirstDayOfWeek")))
    {
        tstring const day = helpers::toUpper (firstDayOfWeekStr);
        if (day == LOG4CPLUS_TEXT("SUNDAY"))
            firstDayOfWeek = FirstDayOfWeek::SUNDAY;
        else if (day == LOG4CPLUS_TEXT("MONDAY"))
            firstDayOfWeek = FirstDayOfWeek::MONDAY;
        else
            helpers::getLogLog().warn (
                LOG4CPLUS_TEXT("DailyRollingFileAppender::ctor()- ")
                LOG4CPLUS_TEXT("\"FirstDayOfWeek\" not valid; using MONDAY: ")
                + firstDayOfWeekStr);
    }

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
    if (firstDayOfWeek != FirstDayOfWeek::SUNDAY
        && firstDayOfWeek != FirstDayOfWeek::MONDAY)
    {
        helpers::getLogLog().warn (
            LOG4CPLUS_TEXT("DailyRollingFileAppender::init()- ")
            LOG4CPLUS_TEXT("invalid FirstDayOfWeek; using MONDAY"));
        firstDayOfWeek = FirstDayOfWeek::MONDAY;
    }
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
calculateNextRolloverTime(const Time& t, DailyRollingFileSchedule schedule,
    FirstDayOfWeek firstDayOfWeek)
{
    namespace chrono = helpers::chrono;

    struct tm next;
    switch(schedule)
    {
    case DailyRollingFileSchedule::MONTHLY:
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

    case DailyRollingFileSchedule::WEEKLY:
    {
        helpers::localTime (&next, t);
        // Round up to the next selected weekday, strictly after t.
        int const days = (7 + static_cast<int> (firstDayOfWeek)
            - next.tm_wday) % 7;
        next.tm_mday += days == 0 ? 7 : days;
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
        [[fallthrough]];

    case DailyRollingFileSchedule::DAILY:
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

    case DailyRollingFileSchedule::TWICE_DAILY:
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

    case DailyRollingFileSchedule::HOURLY:
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

    case DailyRollingFileSchedule::MINUTELY:
        return round_time_and_add (t, chrono::seconds (60));
    };
}


Time
DailyRollingFileAppender::calculateNextRolloverTime(const Time& t) const
{
    return helpers::truncate_fractions (
        log4cplus::calculateNextRolloverTime (t, schedule, firstDayOfWeek));
}


tstring
DailyRollingFileAppender::getFilename(const Time& t) const
{
    tchar const * pattern = nullptr;
    if (datePattern.empty())
    {
        switch (schedule)
        {
        case DailyRollingFileSchedule::MONTHLY:
            pattern = LOG4CPLUS_TEXT("%Y-%m");
            break;

        case DailyRollingFileSchedule::WEEKLY:
            pattern = firstDayOfWeek == FirstDayOfWeek::SUNDAY
                ? LOG4CPLUS_TEXT("%Y-%U") : LOG4CPLUS_TEXT("%Y-%W");
            break;

        default:
            helpers::getLogLog ().error (
                LOG4CPLUS_TEXT ("DailyRollingFileAppender::getFilename()-")
                LOG4CPLUS_TEXT (" invalid schedule value"));
            [[fallthrough]];

        case DailyRollingFileSchedule::DAILY:
            pattern = LOG4CPLUS_TEXT("%Y-%m-%d");
            break;

        case DailyRollingFileSchedule::TWICE_DAILY:
            pattern = LOG4CPLUS_TEXT("%Y-%m-%d-%p");
            break;

        case DailyRollingFileSchedule::HOURLY:
            pattern = LOG4CPLUS_TEXT("%Y-%m-%d-%H");
            break;

        case DailyRollingFileSchedule::MINUTELY:
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
preprocessDateTimePattern(const tstring_view& pattern, DailyRollingFileSchedule& schedule)
{
    // Example: "yyyy-MM-dd HH:mm:ss,aux"
    // Example with space(s) between the ',' and 'aux': "yyyy-MM-dd HH:mm:ss, aux"
    // Patterns from java.text.SimpleDateFormat not implemented here: Y, F, k, K, S, X

    tostringstream result;

    size_t aux_len = 0;
    auto pattern_length = pattern.length();
    if (pattern_length >= 4 && pattern.find(LOG4CPLUS_TEXT("aux"), pattern_length-3) == pattern_length-3) {
        auto const comma_pos = pattern.rfind(LOG4CPLUS_TEXT(","));
        if (comma_pos != tstring_view::npos) {
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
        size_t len = (end_pos == tstring::npos ? pattern_length : end_pos) - i;

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
            schedule = DailyRollingFileSchedule::MINUTELY;
        else if (has_hour)
            schedule = DailyRollingFileSchedule::HOURLY;
        else if (has_day)
            schedule = DailyRollingFileSchedule::DAILY;
        else if (has_week)
            schedule = DailyRollingFileSchedule::WEEKLY;
        else
            schedule = DailyRollingFileSchedule::MONTHLY;
    }

    return result.str();
}

static tstring
preprocessFilenamePattern(const tstring_view& pattern, DailyRollingFileSchedule& schedule)
{
    tostringstream result;

    for (size_t i = 0, pattern_length = pattern.length(); i < pattern_length; )
    {
        tchar c = pattern[i];

        if (c == LOG4CPLUS_TEXT('%') &&
            i < pattern_length-1 &&
            pattern[i+1] == LOG4CPLUS_TEXT('d'))
        {
            if (i < pattern_length-2 && pattern[i+2] == LOG4CPLUS_TEXT('{'))
            {
                size_t closingBracketPos = pattern.find(LOG4CPLUS_TEXT("}"), i+2);
                if (closingBracketPos == std::string::npos)
                {
                    break; // Malformed conversion specifier
                }
                else
                {
                    result << preprocessDateTimePattern(
                        tstring_view (pattern.data () + (i+3),
                            closingBracketPos-(i+3)),
                        schedule);
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
    , schedule(DailyRollingFileSchedule::DAILY)
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
    , schedule(DailyRollingFileSchedule::DAILY)
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

    if (cleanHistoryOnStart) [[unlikely]]
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

    out.open(std::filesystem::path (currentFilename), mode);
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

        if (createDirs)
            internal::make_dirs (scheduledFilename);

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
        if (ret != 0)
        {
            tostringstream oss;
            oss << LOG4CPLUS_TEXT ("Failed to rename file from ")
                << filename
                << LOG4CPLUS_TEXT (" to ")
                << scheduledFilename
                << LOG4CPLUS_TEXT ("; error ")
                << ret;
            getErrorHandler()->error (oss.str ());

            // Preserve the active file and the original archive destination.
            // Leave the deadline unchanged so the next eligible event retries.
            FileAppenderBase::open (std::ios::out | std::ios::app);
            return;
        }
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
    case DailyRollingFileSchedule::MONTHLY:
        return std::chrono::hours{31*24};
    case DailyRollingFileSchedule::WEEKLY:
        return std::chrono::hours{7*24};
    default:
        helpers::getLogLog ().error (
            LOG4CPLUS_TEXT ("TimeBasedRollingFileAppender::getRolloverPeriodDuration()-")
            LOG4CPLUS_TEXT (" invalid schedule value"));
        [[fallthrough]];
    case DailyRollingFileSchedule::DAILY:
        return std::chrono::hours{24};
    case DailyRollingFileSchedule::HOURLY:
        return std::chrono::hours{1};
    case DailyRollingFileSchedule::MINUTELY:
        return std::chrono::minutes{1};
    }
}

Time
TimeBasedRollingFileAppender::calculateNextRolloverTime(const Time& t) const
{
    return helpers::truncate_fractions (
        log4cplus::calculateNextRolloverTime (t, schedule, FirstDayOfWeek::MONDAY));
}


#if defined (LOG4CPLUS_WITH_UNIT_TESTS)
namespace
{

Time
weekly_test_time (int year, int month, int day, int hour = 0,
    int minute = 0, int second = 0)
{
    std::tm date {};
    date.tm_year = year - 1900;
    date.tm_mon = month - 1;
    date.tm_mday = day;
    date.tm_hour = hour;
    date.tm_min = minute;
    date.tm_sec = second;
    date.tm_isdst = -1;
    return helpers::from_struct_tm (&date);
}

struct WeeklyTestDirectory
{
    std::filesystem::path path = std::filesystem::temp_directory_path ()
        / ("log4cplus-weekly-" + std::to_string (
            std::chrono::steady_clock::now ().time_since_epoch ().count ()));

    WeeklyTestDirectory ()
    {
        if (! std::filesystem::create_directory (path))
            throw std::runtime_error ("Cannot create weekly rollover test directory");
    }

    ~WeeklyTestDirectory ()
    {
        std::error_code error;
        std::filesystem::remove_all (path, error);
    }

    tstring filename (char const * name) const
    {
        return LOG4CPLUS_STRING_TO_TSTRING ((path / name).string ());
    }
};

class WeeklyTestAppender : public DailyRollingFileAppender
{
public:
    using DailyRollingFileAppender::DailyRollingFileAppender;
    using DailyRollingFileAppender::calculateNextRolloverTime;
    using DailyRollingFileAppender::getFilename;
    using DailyRollingFileAppender::firstDayOfWeek;
    using DailyRollingFileAppender::nextRolloverTime;
    using DailyRollingFileAppender::scheduledFilename;
};

struct WeeklyWarningCapture
{
    tostringstream output;
    std::basic_streambuf<tchar> * previous = tcerr.rdbuf (output.rdbuf ());

    ~WeeklyWarningCapture ()
    {
        tcerr.rdbuf (previous);
    }
};

Properties
weekly_test_properties (tstring const & filename)
{
    Properties properties;
    properties.setProperty (LOG4CPLUS_TEXT("File"), filename);
    properties.setProperty (LOG4CPLUS_TEXT("Schedule"), LOG4CPLUS_TEXT("WEEKLY"));
    properties.setProperty (LOG4CPLUS_TEXT("RollOnClose"), LOG4CPLUS_TEXT("false"));
    return properties;
}

} // namespace

CATCH_TEST_CASE ("Weekly rollover from Sunday uses the following Monday",
    "[appender][weekly]")
{
    std::tm date {};
    date.tm_year = 126;
    date.tm_mon = 9;
    date.tm_mday = 4; // Sunday, October 4, 2026.
    date.tm_hour = 12;
    date.tm_isdst = -1;
    Time const sunday = helpers::from_struct_tm (&date);
    date.tm_mday = 5;
    date.tm_hour = 0;
    date.tm_isdst = -1;
    Time const monday = helpers::from_struct_tm (&date);

    CATCH_CHECK (calculateNextRolloverTime (sunday,
        DailyRollingFileSchedule::WEEKLY, FirstDayOfWeek::MONDAY) == monday);
}

CATCH_TEST_CASE ("Weekly rollover selects the next Sunday or Monday",
    "[appender][weekly]")
{
    int const monday_dates[] = {5, 12, 12, 12, 12, 12, 12};
    for (FirstDayOfWeek first : {FirstDayOfWeek::SUNDAY, FirstDayOfWeek::MONDAY})
        for (int weekday = 0; weekday < 7; ++weekday)
            for (int hour : {0, 12})
            {
                CATCH_CAPTURE (first, weekday, hour);
                Time const input = weekly_test_time (2026, 10, 4 + weekday, hour);
                Time const expected = weekly_test_time (2026, 10,
                    first == FirstDayOfWeek::SUNDAY ? 11 : monday_dates[weekday]);
                CATCH_CHECK (calculateNextRolloverTime (input,
                    DailyRollingFileSchedule::WEEKLY, first) == expected);
                CATCH_CHECK (expected > input);
            }

    for (FirstDayOfWeek first : {FirstDayOfWeek::SUNDAY, FirstDayOfWeek::MONDAY})
    {
        Time const boundary = weekly_test_time (2026, 10,
            first == FirstDayOfWeek::SUNDAY ? 4 : 5);
        CATCH_CHECK (calculateNextRolloverTime (
            boundary - std::chrono::microseconds {1},
            DailyRollingFileSchedule::WEEKLY, first) == boundary);
        CATCH_CHECK (calculateNextRolloverTime (
            boundary + std::chrono::microseconds {1},
            DailyRollingFileSchedule::WEEKLY, first)
            == weekly_test_time (2026, 10,
                first == FirstDayOfWeek::SUNDAY ? 11 : 12));
    }
}

CATCH_TEST_CASE ("Weekly rollover crosses calendar and DST boundaries",
    "[appender][weekly]")
{
    struct Scenario
    {
        FirstDayOfWeek first;
        int year, month, day;
        int next_year, next_month, next_day;
    };
    Scenario const scenarios[] = {
        {FirstDayOfWeek::SUNDAY, 2024, 2, 25, 2024, 3, 3},
        {FirstDayOfWeek::MONDAY, 2024, 2, 26, 2024, 3, 4},
        {FirstDayOfWeek::SUNDAY, 2026, 1, 31, 2026, 2, 1},
        {FirstDayOfWeek::MONDAY, 2026, 1, 31, 2026, 2, 2},
        {FirstDayOfWeek::SUNDAY, 2026, 12, 31, 2027, 1, 3},
        {FirstDayOfWeek::MONDAY, 2026, 12, 31, 2027, 1, 4},
        // Run in a timezone with European DST rules as well as UTC.
        {FirstDayOfWeek::SUNDAY, 2026, 3, 29, 2026, 4, 5},
        {FirstDayOfWeek::MONDAY, 2026, 3, 23, 2026, 3, 30},
        {FirstDayOfWeek::SUNDAY, 2026, 10, 25, 2026, 11, 1},
        {FirstDayOfWeek::MONDAY, 2026, 10, 19, 2026, 10, 26},
    };
    for (auto const & scenario : scenarios)
    {
        CATCH_CAPTURE (scenario.first, scenario.year, scenario.month, scenario.day);
        CATCH_CHECK (calculateNextRolloverTime (
            weekly_test_time (scenario.year, scenario.month, scenario.day),
            DailyRollingFileSchedule::WEEKLY, scenario.first)
            == weekly_test_time (scenario.next_year, scenario.next_month,
                scenario.next_day));
    }
}

CATCH_TEST_CASE ("Weekly appender properties and constructor select the same weekday",
    "[appender][weekly]")
{
    WeeklyTestDirectory directory;
    struct Setting
    {
        tchar const * name;
        FirstDayOfWeek first;
    };
    Setting const settings[] = {
        {LOG4CPLUS_TEXT("SUNDAY"), FirstDayOfWeek::SUNDAY},
        {LOG4CPLUS_TEXT("sUnDaY"), FirstDayOfWeek::SUNDAY},
        {LOG4CPLUS_TEXT("MONDAY"), FirstDayOfWeek::MONDAY},
        {LOG4CPLUS_TEXT("monday"), FirstDayOfWeek::MONDAY},
    };
    for (auto const & setting : settings)
    {
        CATCH_CAPTURE (setting.name);
        Properties properties = weekly_test_properties (directory.filename ("properties.log"));
        properties.setProperty (LOG4CPLUS_TEXT("FirstDayOfWeek"), setting.name);
        WeeklyWarningCapture warnings;
        WeeklyTestAppender configured (properties);
        WeeklyTestAppender direct (directory.filename ("direct.log"),
            DailyRollingFileSchedule::WEEKLY, true, 10, false, false,
            tstring (), setting.first);
        CATCH_CHECK (warnings.output.str ().empty ());
        CATCH_CHECK (configured.firstDayOfWeek == setting.first);
        CATCH_CHECK (direct.firstDayOfWeek == setting.first);
        std::tm next;
        helpers::localTime (&next, configured.nextRolloverTime);
        CATCH_CHECK (next.tm_wday == static_cast<int> (setting.first));
        helpers::localTime (&next, direct.nextRolloverTime);
        CATCH_CHECK (next.tm_wday == static_cast<int> (setting.first));
        Time const input = weekly_test_time (2026, 10, 4, 12);
        Time const expected = weekly_test_time (2026, 10,
            setting.first == FirstDayOfWeek::SUNDAY ? 11 : 5);
        CATCH_CHECK (configured.calculateNextRolloverTime (input) == expected);
        CATCH_CHECK (direct.calculateNextRolloverTime (input) == expected);
        tstring const suffix = setting.first == FirstDayOfWeek::SUNDAY
            ? LOG4CPLUS_TEXT(".2026-40") : LOG4CPLUS_TEXT(".2026-39");
        CATCH_CHECK (configured.getFilename (input)
            == directory.filename ("properties.log") + suffix);
        CATCH_CHECK (direct.getFilename (input)
            == directory.filename ("direct.log") + suffix);
    }

    Properties properties = weekly_test_properties (directory.filename ("default-properties.log"));
    WeeklyTestAppender configured (properties);
    // Existing constructor calls can still omit the weekday argument.
    WeeklyTestAppender direct (directory.filename ("default-direct.log"),
        DailyRollingFileSchedule::WEEKLY, true, 10, false, false);
    CATCH_CHECK (configured.firstDayOfWeek == FirstDayOfWeek::MONDAY);
    CATCH_CHECK (direct.firstDayOfWeek == FirstDayOfWeek::MONDAY);
}

CATCH_TEST_CASE ("Invalid first weekdays warn and fall back to Monday",
    "[appender][weekly]")
{
    WeeklyTestDirectory directory;
    for (tchar const * value : {LOG4CPLUS_TEXT("SATURDAY"), LOG4CPLUS_TEXT("SUN"),
        LOG4CPLUS_TEXT("0"), LOG4CPLUS_TEXT("1"), LOG4CPLUS_TEXT(""),
        LOG4CPLUS_TEXT("unknown")})
    {
        CATCH_CAPTURE (value);
        Properties properties = weekly_test_properties (directory.filename ("invalid.log"));
        properties.setProperty (LOG4CPLUS_TEXT("FirstDayOfWeek"), value);
        WeeklyWarningCapture warnings;
        WeeklyTestAppender appender (properties);
        CATCH_CHECK (appender.firstDayOfWeek == FirstDayOfWeek::MONDAY);
        CATCH_CHECK (appender.calculateNextRolloverTime (weekly_test_time (2026, 10, 4))
            == weekly_test_time (2026, 10, 5));
        CATCH_CHECK (appender.getFilename (weekly_test_time (2026, 10, 4))
            == directory.filename ("invalid.log") + LOG4CPLUS_TEXT(".2026-39"));
        CATCH_CHECK (warnings.output.str ().find (LOG4CPLUS_TEXT("WARN")) != tstring::npos);
        CATCH_CHECK (warnings.output.str ().find (LOG4CPLUS_TEXT("FirstDayOfWeek")) != tstring::npos);
    }
    for (int value : {-1, 2, 6})
    {
        CATCH_CAPTURE (value);
        WeeklyWarningCapture warnings;
        WeeklyTestAppender appender (directory.filename ("invalid-enum.log"),
            DailyRollingFileSchedule::WEEKLY, true, 10, false, false,
            tstring (), static_cast<FirstDayOfWeek> (value));
        CATCH_CHECK (appender.firstDayOfWeek == FirstDayOfWeek::MONDAY);
        CATCH_CHECK (appender.calculateNextRolloverTime (weekly_test_time (2026, 10, 4))
            == weekly_test_time (2026, 10, 5));
        CATCH_CHECK (warnings.output.str ().find (LOG4CPLUS_TEXT("WARN")) != tstring::npos);
    }
}

CATCH_TEST_CASE ("Weekly archive names follow the configured week numbering",
    "[appender][weekly]")
{
    WeeklyTestDirectory directory;
    for (FirstDayOfWeek first : {FirstDayOfWeek::SUNDAY, FirstDayOfWeek::MONDAY})
    {
        CATCH_CAPTURE (first);
        auto const filename = directory.filename ("weekly.log");
        WeeklyTestAppender appender (filename, DailyRollingFileSchedule::WEEKLY,
            true, 10, false, false, tstring (), first);
        int const start_day = first == FirstDayOfWeek::SUNDAY ? 4 : 5;
        for (int offset = 0; offset < 7; ++offset)
            CATCH_CHECK (appender.getFilename (weekly_test_time (2026, 10,
                start_day + offset, 12)) == filename + LOG4CPLUS_TEXT(".2026-40"));
        Time const boundary = weekly_test_time (2026, 10, start_day + 7);
        CATCH_CHECK (appender.getFilename (boundary - std::chrono::microseconds {1})
            == filename + LOG4CPLUS_TEXT(".2026-40"));
        CATCH_CHECK (appender.getFilename (boundary)
            == filename + LOG4CPLUS_TEXT(".2026-41"));
        CATCH_CHECK (appender.getFilename (weekly_test_time (2026, 1, 1))
            == filename + LOG4CPLUS_TEXT(".2026-00"));
        CATCH_CHECK (appender.getFilename (weekly_test_time (2026, 1, 4))
            == filename + (first == FirstDayOfWeek::SUNDAY
                ? LOG4CPLUS_TEXT(".2026-01") : LOG4CPLUS_TEXT(".2026-00")));
        CATCH_CHECK (appender.getFilename (weekly_test_time (2026, 1, 5))
            == filename + LOG4CPLUS_TEXT(".2026-01"));

        Properties properties = weekly_test_properties (directory.filename ("custom.log"));
        properties.setProperty (LOG4CPLUS_TEXT("FirstDayOfWeek"),
            first == FirstDayOfWeek::SUNDAY ? LOG4CPLUS_TEXT("SUNDAY") : LOG4CPLUS_TEXT("MONDAY"));
        properties.setProperty (LOG4CPLUS_TEXT("DatePattern"), LOG4CPLUS_TEXT("%Y-%U-%W-%%W"));
        WeeklyTestAppender custom (properties);
        CATCH_CHECK (custom.getFilename (weekly_test_time (2026, 10, 4))
            == directory.filename ("custom.log") + LOG4CPLUS_TEXT(".2026-40-39-%W"));
    }
}

CATCH_TEST_CASE ("First weekday does not affect other rollover schedules",
    "[appender][weekly]")
{
    WeeklyTestDirectory directory;
    Time const input = weekly_test_time (2026, 10, 4, 10, 34, 56);
    for (auto schedule : {DailyRollingFileSchedule::MONTHLY,
        DailyRollingFileSchedule::DAILY, DailyRollingFileSchedule::TWICE_DAILY,
        DailyRollingFileSchedule::HOURLY, DailyRollingFileSchedule::MINUTELY})
    {
        CATCH_CAPTURE (schedule);
        WeeklyTestAppender sunday (directory.filename ("other.log"), schedule,
            true, 10, false, false, tstring (), FirstDayOfWeek::SUNDAY);
        WeeklyTestAppender monday (directory.filename ("other.log"), schedule,
            true, 10, false, false, tstring (), FirstDayOfWeek::MONDAY);
        CATCH_CHECK (sunday.calculateNextRolloverTime (input)
            == monday.calculateNextRolloverTime (input));
        CATCH_CHECK (sunday.getFilename (input) == monday.getFilename (input));
    }
}

CATCH_TEST_CASE ("Time-based weekly rollover retains Monday",
    "[appender][weekly]")
{
    class TestAppender : public TimeBasedRollingFileAppender
    {
    public:
        using TimeBasedRollingFileAppender::TimeBasedRollingFileAppender;
        using TimeBasedRollingFileAppender::calculateNextRolloverTime;
    };
    WeeklyTestDirectory directory;
    TestAppender appender (directory.filename ("current.log"),
        directory.filename ("archive-%d{yyyy-ww}.log"), 10, false, true, false, false);
    CATCH_CHECK (appender.calculateNextRolloverTime (weekly_test_time (2026, 10, 4))
        == weekly_test_time (2026, 10, 5));
    CATCH_CHECK (appender.calculateNextRolloverTime (weekly_test_time (2026, 10, 5))
        == weekly_test_time (2026, 10, 12));
}

CATCH_TEST_CASE ("Weekly appenders rotate on the first event at the deadline",
    "[appender][weekly]")
{
    class TestEvent : public spi::InternalLoggingEvent
    {
    public:
        TestEvent (tchar const * message, Time time)
            : InternalLoggingEvent (LOG4CPLUS_TEXT("weekly"), INFO_LOG_LEVEL,
                message, nullptr, 0)
        {
            timestamp = time;
        }
    };
    for (FirstDayOfWeek first : {FirstDayOfWeek::SUNDAY, FirstDayOfWeek::MONDAY})
    {
        CATCH_CAPTURE (first);
        WeeklyTestDirectory directory;
        auto const filename = directory.filename ("current.log");
        WeeklyTestAppender appender (filename, DailyRollingFileSchedule::WEEKLY,
            true, 10, false, false, tstring (), first);
        Time const boundary = weekly_test_time (2024, 1,
            first == FirstDayOfWeek::SUNDAY ? 7 : 8);
        Time const before = boundary - std::chrono::microseconds {1};
        appender.nextRolloverTime = boundary;
        appender.scheduledFilename = appender.getFilename (before);
        std::filesystem::path const archive (appender.scheduledFilename);
        appender.doAppend (TestEvent (LOG4CPLUS_TEXT("before"), before));
        CATCH_CHECK_FALSE (std::filesystem::exists (archive));
        appender.doAppend (TestEvent (LOG4CPLUS_TEXT("at"), boundary));
        CATCH_CHECK (std::filesystem::exists (archive));
        CATCH_CHECK (appender.nextRolloverTime > helpers::now ());
        std::tm next;
        helpers::localTime (&next, appender.nextRolloverTime);
        CATCH_CHECK (next.tm_wday == static_cast<int> (first));
        appender.doAppend (TestEvent (LOG4CPLUS_TEXT("after"),
            boundary + std::chrono::microseconds {1}));
        appender.close ();

        auto const contents = [] (std::filesystem::path const & path)
        {
            std::ifstream file (path);
            CATCH_REQUIRE (file.good ());
            std::ostringstream output;
            output << file.rdbuf ();
            return output.str ();
        };
        CATCH_CHECK (contents (archive) == "INFO - before\n");
        CATCH_CHECK (contents (std::filesystem::path (filename))
            == "INFO - at\nINFO - after\n");
        CATCH_CHECK_FALSE (std::filesystem::exists (
            std::filesystem::path (archive.string () + ".1")));
    }
}


namespace
{

void
check_file_layout_eol(bool unicode_separators)
{
    // Some standard libraries only provide the C locale. Require UTF-8
    // conversion only where the stream's character type makes it necessary.
    std::optional<std::locale> utf8_locale;
#if (defined (UNICODE) && !defined (_WIN32)) \
    || (!defined (UNICODE) && defined (_WIN32))
    if (unicode_separators)
    {
        for (auto name : {".UTF-8", "C.UTF-8", "en_US.UTF-8"})
        {
            try
            {
                utf8_locale.emplace(name);
                break;
            }
            catch (std::runtime_error const &) { }
        }
        if (! utf8_locale)
            CATCH_SKIP("No UTF-8 stream locale available for Unicode EOL file output");
    }
#endif

    bool const binary = GENERATE(false, true);
    int const kind = GENERATE(0, 1, 2);
    bool const daily = GENERATE(false, true);
    CATCH_CAPTURE(binary, kind, daily, unicode_separators);

    struct TestDirectory
    {
        std::filesystem::path path = std::filesystem::temp_directory_path()
            / ("log4cplus-eol-" + std::to_string(
                std::chrono::steady_clock::now().time_since_epoch().count()));
        TestDirectory() { std::filesystem::create_directory(path); }
        ~TestDirectory()
        {
            std::error_code error;
            std::filesystem::remove_all(path, error);
        }
    } directory;
    auto const path = directory.path / "output.log";
    Properties properties;
    properties.setProperty(LOG4CPLUS_TEXT("File"), LOG4CPLUS_STRING_TO_TSTRING(path.string()));
    properties.setProperty(LOG4CPLUS_TEXT("Append"), LOG4CPLUS_TEXT("false"));
    properties.setProperty(LOG4CPLUS_TEXT("Schedule"), LOG4CPLUS_TEXT("DAILY"));
    properties.setProperty(LOG4CPLUS_TEXT("RollOnClose"), LOG4CPLUS_TEXT("false"));
    if (binary)
        properties.setProperty(LOG4CPLUS_TEXT("TextMode"), LOG4CPLUS_TEXT("Binary"));
    tchar const * const layouts[] = {
        LOG4CPLUS_TEXT("log4cplus::SimpleLayout"),
        LOG4CPLUS_TEXT("log4cplus::TTCCLayout"),
        LOG4CPLUS_TEXT("log4cplus::PatternLayout"),
    };
    properties.setProperty(LOG4CPLUS_TEXT("layout"), layouts[kind]);
    properties.setProperty(LOG4CPLUS_TEXT("layout.ConversionPattern"), LOG4CPLUS_TEXT("%m%n"));
    properties.setProperty(LOG4CPLUS_TEXT("layout.DateFormat"), LOG4CPLUS_TEXT("time"));
    properties.setProperty(LOG4CPLUS_TEXT("layout.ThreadPrinting"), LOG4CPLUS_TEXT("false"));
    properties.setProperty(LOG4CPLUS_TEXT("layout.CategoryPrefixing"), LOG4CPLUS_TEXT("false"));
    properties.setProperty(LOG4CPLUS_TEXT("layout.ContextPrinting"), LOG4CPLUS_TEXT("false"));
    spi::InternalLoggingEvent event(LOG4CPLUS_TEXT("logger"), INFO_LOG_LEVEL,
        LOG4CPLUS_TEXT("a\nb\r\nc\rd"), nullptr, 0);

    struct
    {
        tchar const * name;
        char const * bytes;
    } const cases[] = {
        {nullptr, "\n"},
        {LOG4CPLUS_TEXT("CR"), "\r"},
        {LOG4CPLUS_TEXT("LF"), "\n"},
        {LOG4CPLUS_TEXT("CRLF"), "\r\n"},
        {LOG4CPLUS_TEXT("NEL"), "\xC2\x85"},
        {LOG4CPLUS_TEXT("LS"), "\xE2\x80\xA8"},
        {LOG4CPLUS_TEXT("PS"), "\xE2\x80\xA9"},
    };
    for (int index = unicode_separators ? 4 : 0;
        index < (unicode_separators ? 7 : 4); ++index)
    {
        auto const & item = cases[index];
        CATCH_CAPTURE(index);
        if (item.name)
            properties.setProperty(LOG4CPLUS_TEXT("layout.EOL"), item.name);
        std::unique_ptr<FileAppenderBase> appender;
        if (daily)
            appender = std::make_unique<DailyRollingFileAppender>(properties);
        else
            appender = std::make_unique<FileAppender>(properties);
        if (utf8_locale)
            appender->imbue(*utf8_locale);
        appender->doAppend(event);
        appender->close();

        std::ifstream file(path, std::ios_base::binary);
        CATCH_REQUIRE(file.is_open());
        std::string const actual((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        std::string expected = kind == 0 ? "INFO - " : kind == 1 ? "time INFO - " : "";
        expected += "a\nb\r\nc\rd";
        expected += item.bytes;
#if defined (_WIN32)
        if (! binary)
        {
            std::string translated;
            for (char ch : expected)
            {
                if (ch == '\n')
                    translated += '\r';
                translated += ch;
            }
            expected = std::move(translated);
        }
#endif
        CATCH_CHECK(actual == expected);
    }
}


} // namespace

CATCH_TEST_CASE("File appenders preserve layout EOL and text-mode behavior", "[appender][layout][eol]")
{
    check_file_layout_eol(false);
}

CATCH_TEST_CASE("File appenders encode Unicode layout separators", "[appender][layout][eol][unicode]")
{
    check_file_layout_eol(true);
}


CATCH_TEST_CASE ("TimeBasedRollingFileAppender", "[appender]")
{

    CATCH_SECTION ("date format string preprocessing")
    {
        DailyRollingFileSchedule schedule;
        CATCH_REQUIRE (preprocessDateTimePattern(
            LOG4CPLUS_TEXT ("yyyy-MM-dd"), schedule)
            == LOG4CPLUS_TEXT ("%Y-%m-%d"));
        CATCH_REQUIRE (schedule == DailyRollingFileSchedule::DAILY);
    }

    CATCH_SECTION ("file name pattern preprocessing")
    {
        DailyRollingFileSchedule schedule;
        CATCH_REQUIRE (preprocessFilenamePattern(
            LOG4CPLUS_TEXT ("log-%d{yyyy-MM-dd}"), schedule)
            == LOG4CPLUS_TEXT ("log-%Y-%m-%d"));
        CATCH_REQUIRE (schedule == DailyRollingFileSchedule::DAILY);
    }

}

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

CATCH_TEST_CASE ("TimeBasedRollingFileAppender creates dated archive directories",
    "[appender][timebased-rollover][timebased-archive-dirs]")
{
    bool const direct = GENERATE (false, true);
    bool const existing = GENERATE (false, true);
    CATCH_CAPTURE (direct, existing);
    TimeBasedTestClock clock;
    TimeBasedTestDirectory directory;
    auto const current = directory.file (LOG4CPLUS_TEXT ("active/current.log"));
    auto const first = directory.file (
        LOG4CPLUS_TEXT ("archive/20251114/MyApplication/log.txt"));
    auto const second = directory.file (
        LOG4CPLUS_TEXT ("archive/20251115/MyApplication/log.txt"));
    auto const third = directory.file (
        LOG4CPLUS_TEXT ("archive/20251116/MyApplication/log.txt"));
    auto const pattern = directory.path
        + LOG4CPLUS_TEXT ("/archive/%d{yyyyMMdd}/MyApplication/log.txt");
    if (existing)
    {
        internal::make_dirs (first);
        internal::make_dirs (second);
    }
    auto properties = timeBasedTestProperties (pattern);
    properties.setProperty (LOG4CPLUS_TEXT ("File"), current);
    std::unique_ptr<TimeBasedTestAppender> appender;
    if (direct)
        appender.reset (new TimeBasedTestAppender (
            current, pattern, 365, false, true, true, false));
    else
        appender.reset (new TimeBasedTestAppender (properties));
    timeBasedTestLayout (*appender);
    auto * errors = new TimeBasedTestErrorHandler;
    appender->setErrorHandler (std::unique_ptr<ErrorHandler> (errors));

    appender->log (LOG4CPLUS_TEXT ("one"), clock.now);
    CATCH_CHECK (directory.read (current) == "one\n");
    CATCH_CHECK (! directory.exists (first));
    clock.now = timeBasedTestDate (15);
    appender->log (LOG4CPLUS_TEXT ("two"), clock.now);
    CATCH_CHECK (directory.read (first) == "one\n");
    CATCH_CHECK (directory.read (current) == "two\n");
    CATCH_CHECK (! directory.exists (second));
    clock.now = timeBasedTestDate (16);
    appender->log (LOG4CPLUS_TEXT ("three"), clock.now);
    CATCH_CHECK (directory.read (first) == "one\n");
    CATCH_CHECK (directory.read (second) == "two\n");
    CATCH_CHECK (directory.read (current) == "three\n");
    CATCH_CHECK (! directory.exists (third));
    CATCH_CHECK (errors->errors.empty ());
}

CATCH_TEST_CASE ("TimeBasedRollingFileAppender preserves active files after failed archiving",
    "[appender][timebased-rollover][timebased-archive-dirs]")
{
    bool const blocked = GENERATE (false, true);
    CATCH_CAPTURE (blocked);
    TimeBasedTestClock clock;
    TimeBasedTestDirectory directory;
    auto const current = directory.file (LOG4CPLUS_TEXT ("current.log"));
    auto const archiveRoot = directory.file (LOG4CPLUS_TEXT ("archive"));
    auto const first = directory.file (
        LOG4CPLUS_TEXT ("archive/20251114/MyApplication/log.txt"));
    auto const second = directory.file (
        LOG4CPLUS_TEXT ("archive/20251115/MyApplication/log.txt"));
    if (blocked)
        directory.write (archiveRoot, "blocks directory creation\n");
    auto properties = timeBasedTestProperties (directory.path
        + LOG4CPLUS_TEXT ("/archive/%d{yyyyMMdd}/MyApplication/log.txt"));
    properties.setProperty (LOG4CPLUS_TEXT ("File"), current);
    properties.setProperty (LOG4CPLUS_TEXT ("CreateDirs"),
        blocked ? LOG4CPLUS_TEXT ("true") : LOG4CPLUS_TEXT ("false"));
    TimeBasedTestAppender appender (properties);
    timeBasedTestLayout (appender);
    auto * errors = new TimeBasedTestErrorHandler;
    appender.setErrorHandler (std::unique_ptr<ErrorHandler> (errors));
    appender.log (LOG4CPLUS_TEXT ("one"), clock.now);
    clock.now = timeBasedTestDate (15);
    appender.log (LOG4CPLUS_TEXT ("two"), clock.now);
    CATCH_CHECK (directory.read (current) == "one\ntwo\n");
    CATCH_CHECK (! directory.exists (first));
    CATCH_CHECK (! directory.exists (second));
    CATCH_REQUIRE (errors->errors.size () == 1);
    tstring const errorPrefix = LOG4CPLUS_TEXT ("Failed to rename file from ")
        + current + LOG4CPLUS_TEXT (" to ") + first
        + LOG4CPLUS_TEXT ("; error ");
    CATCH_CHECK (errors->errors[0].find (errorPrefix) == 0);
    CATCH_CHECK (errors->errors[0].size () > errorPrefix.size ());
    if (! blocked)
    {
        CATCH_CHECK (! directory.exists (archiveRoot));
        CATCH_CHECK (errors->errors[0] == errorPrefix
            + helpers::convertIntegerToString (ENOENT));
    }

    appender.log (LOG4CPLUS_TEXT ("three"), clock.now);
    CATCH_CHECK (directory.read (current) == "one\ntwo\nthree\n");
    CATCH_REQUIRE (errors->errors.size () == 2);
    CATCH_CHECK (errors->errors[1] == errors->errors[0]);
    if (blocked)
        CATCH_REQUIRE (file_remove (archiveRoot) == 0);
    else
        internal::make_dirs (first);

    // Retry at the same timestamp, still targeting the original period.
    appender.log (LOG4CPLUS_TEXT ("four"), clock.now);
    CATCH_CHECK (directory.read (first) == "one\ntwo\nthree\n");
    CATCH_CHECK (directory.read (current) == "four\n");
    CATCH_CHECK (! directory.exists (second));
    CATCH_CHECK (errors->errors.size () == 2);
    if (! blocked)
        internal::make_dirs (second);
    clock.now = timeBasedTestDate (16);
    appender.log (LOG4CPLUS_TEXT ("five"), clock.now);
    CATCH_CHECK (directory.read (first) == "one\ntwo\nthree\n");
    CATCH_CHECK (directory.read (second) == "four\n");
    CATCH_CHECK (directory.read (current) == "five\n");
    CATCH_CHECK (errors->errors.size () == 2);
}

CATCH_TEST_CASE ("TimeBasedRollingFileAppender safely archives dated directories on close",
    "[appender][timebased-rollover][timebased-archive-dirs]")
{
    int const scenario = GENERATE (0, 1, 2);
    CATCH_CAPTURE (scenario);
    TimeBasedTestClock clock;
    TimeBasedTestDirectory directory;
    auto const current = directory.file (LOG4CPLUS_TEXT ("current.log"));
    auto const archiveRoot = directory.file (LOG4CPLUS_TEXT ("archive"));
    auto const archive = directory.file (
        LOG4CPLUS_TEXT ("archive/20251114/MyApplication/log.txt"));
    if (scenario == 2)
        directory.write (archiveRoot, "blocks directory creation\n");
    auto properties = timeBasedTestProperties (directory.path
        + LOG4CPLUS_TEXT ("/archive/%d{yyyyMMdd}/MyApplication/log.txt"));
    properties.setProperty (LOG4CPLUS_TEXT ("File"), current);
    properties.setProperty (LOG4CPLUS_TEXT ("CreateDirs"),
        scenario == 1 ? LOG4CPLUS_TEXT ("false") : LOG4CPLUS_TEXT ("true"));
    properties.setProperty (LOG4CPLUS_TEXT ("RollOnClose"), LOG4CPLUS_TEXT ("true"));
    TimeBasedTestAppender appender (properties);
    timeBasedTestLayout (appender);
    auto * errors = new TimeBasedTestErrorHandler;
    appender.setErrorHandler (std::unique_ptr<ErrorHandler> (errors));
    appender.log (LOG4CPLUS_TEXT ("one"), clock.now);
    CATCH_CHECK (! directory.exists (archive));
    appender.close ();
    CATCH_CHECK (appender.isClosed ());
    if (scenario == 0)
    {
        CATCH_CHECK (directory.read (archive) == "one\n");
        CATCH_CHECK (directory.read (current).empty ());
        CATCH_CHECK (errors->errors.empty ());
    }
    else
    {
        CATCH_CHECK (directory.read (current) == "one\n");
        CATCH_CHECK (! directory.exists (archive));
        CATCH_REQUIRE (errors->errors.size () == 1);
        CATCH_CHECK (errors->errors[0].find (
            LOG4CPLUS_TEXT ("Failed to rename file from ") + current
            + LOG4CPLUS_TEXT (" to ") + archive + LOG4CPLUS_TEXT ("; error ")) == 0);
        if (scenario == 1)
            CATCH_CHECK (! directory.exists (archiveRoot));
    }
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
        std::filesystem::path path = std::filesystem::temp_directory_path ()
            / ("log4cplus-clean-" + std::to_string (
                std::chrono::steady_clock::now ().time_since_epoch ().count ()));

        TestDirectory ()
        {
            if (! std::filesystem::create_directory (path))
                throw std::runtime_error ("Cannot create cleanup test directory");
        }

        ~TestDirectory ()
        {
            std::error_code error;
            std::filesystem::remove_all (path, error);
        }
    } directory;

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
    auto const prefix = LOG4CPLUS_STRING_TO_TSTRING (directory.path.string ())
        + LOG4CPLUS_TEXT ("/");
    TestAppender appender (prefix + LOG4CPLUS_TEXT ("current.log"),
        prefix + LOG4CPLUS_TEXT ("archive-%d{yyyy-MM-dd_HH-mm}.log"),
        2, false, true, false, false);
    auto const archive_name = [&] (int minute)
    {
        return std::filesystem::path (prefix + helpers::getFormattedTime (
            LOG4CPLUS_TEXT ("archive-%Y-%m-%d_%H-%M.log"),
            start + std::chrono::minutes {minute}, false));
    };
    int const first_minute = previous_seconds / 60 - 2;
    int const last_minute = std::max (previous_seconds, current_seconds) / 60 + 1;
    for (int i = first_minute; i <= last_minute; ++i)
    {
        std::ofstream archive (archive_name (i));
        archive << "archive " << i;
        CATCH_REQUIRE (archive.good ());
    }
    auto const unrelated = directory.path / "unrelated.log";
    std::ofstream (unrelated) << "unrelated";

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
            CATCH_CHECK (std::filesystem::exists (archive_name (i)) == retained);
        }
    }
    CATCH_CHECK (std::filesystem::exists (unrelated));
    CATCH_CHECK (std::filesystem::exists (
        std::filesystem::path (prefix + LOG4CPLUS_TEXT ("current.log"))));
}
#endif


} // namespace log4cplus
