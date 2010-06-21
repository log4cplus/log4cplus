// Module:  Log4CPLUS
// File:    fileappender.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright 2001-2010 Tad E. Smith
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
#include <log4cplus/spi/loggingevent.h>
#include <algorithm>
#include <cstdio>
#if defined (__BORLANDC__)
// For _wrename() and _wremove() on Windows.
#  include <stdio.h>
#endif
#if ! defined (_WIN32_WCE)
#include <cerrno>
#endif
#include <cstdlib>

#if defined (_WIN32_WCE)
#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif


namespace log4cplus
{

using helpers::Properties;
using helpers::Time;


const long MINIMUM_ROLLING_LOG_SIZE = 200*1024L;


///////////////////////////////////////////////////////////////////////////////
// File LOCAL definitions
///////////////////////////////////////////////////////////////////////////////

namespace
{


#if defined (_WIN32_WCE)
long const LOG4CPLUS_FILE_NOT_FOUND = ERROR_FILE_NOT_FOUND;
#else
long const LOG4CPLUS_FILE_NOT_FOUND = ENOENT;
#endif


static 
long
file_rename (tstring const & src, tstring const & target)
{
#if defined (_WIN32_WCE)
    if (MoveFile (src.c_str (), target.c_str ()))
        return 0;
    else
        return GetLastError ();

#elif defined (UNICODE) && defined (WIN32)
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
#if defined (_WIN32_WCE)
    if (DeleteFile (src.c_str ()))
        return 0;
    else
        return GetLastError ();

#elif defined (UNICODE) && defined (WIN32)
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
            << target
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
    helpers::SharedObjectPtr<helpers::LogLog> loglog
        = helpers::LogLog::getLogLog();

    // Delete the oldest file
    tostringstream buffer;
    buffer << filename << LOG4CPLUS_TEXT(".") << maxBackupIndex;
    long ret = file_remove (buffer.str ());

    tostringstream source_oss;
    tostringstream target_oss;

    // Map {(maxBackupIndex - 1), ..., 2, 1} to {maxBackupIndex, ..., 3, 2}
    for (int i = maxBackupIndex - 1; i >= 1; --i)
    {
        source_oss.str(LOG4CPLUS_TEXT(""));
        target_oss.str(LOG4CPLUS_TEXT(""));

        source_oss << filename << LOG4CPLUS_TEXT(".") << i;
        target_oss << filename << LOG4CPLUS_TEXT(".") << (i+1);

        tstring const source (source_oss.str ());
        tstring const target (target_oss.str ());

#if defined (WIN32)
        // Try to remove the target first. It seems it is not
        // possible to rename over existing file.
        ret = file_remove (target);
#endif

        ret = file_rename (source, target);
        loglog_renaming_result (*loglog, source, target, ret);
    }
} // end rolloverFiles()

}


///////////////////////////////////////////////////////////////////////////////
// FileAppender ctors and dtor
///////////////////////////////////////////////////////////////////////////////

FileAppender::FileAppender(const tstring& filename_, 
    LOG4CPLUS_OPEN_MODE_TYPE mode, bool immediateFlush_)
    : immediateFlush(immediateFlush_)
    , reopenDelay(1)
    , bufferSize (0)
    , buffer (0)
{
    init(filename_, mode);
}


FileAppender::FileAppender(const Properties& properties, 
                           LOG4CPLUS_OPEN_MODE_TYPE mode)
    : Appender(properties)
    , immediateFlush(true)
    , reopenDelay(1)
    , bufferSize (0)
    , buffer (0)
{
    bool append_ = (mode == std::ios::app);
    tstring filename_ = properties.getProperty( LOG4CPLUS_TEXT("File") );
    if (filename_.empty())
    {
        getErrorHandler()->error( LOG4CPLUS_TEXT("Invalid filename") );
        return;
    }
    if(properties.exists( LOG4CPLUS_TEXT("ImmediateFlush") )) {
        tstring tmp = properties.getProperty( LOG4CPLUS_TEXT("ImmediateFlush") );
        immediateFlush = (helpers::toLower(tmp) == LOG4CPLUS_TEXT("true"));
    }
    if(properties.exists( LOG4CPLUS_TEXT("Append") )) {
        tstring tmp = properties.getProperty( LOG4CPLUS_TEXT("Append") );
        append_ = (helpers::toLower(tmp) == LOG4CPLUS_TEXT("true"));
    }
    if(properties.exists( LOG4CPLUS_TEXT("ReopenDelay") )) {
        tstring tmp = properties.getProperty( LOG4CPLUS_TEXT("ReopenDelay") );
        reopenDelay = std::atoi(LOG4CPLUS_TSTRING_TO_STRING(tmp).c_str());
    }
    if(properties.exists( LOG4CPLUS_TEXT("BufferSize") )) {
        tstring tmp = properties.getProperty( LOG4CPLUS_TEXT("BufferSize") );
        bufferSize = std::atoi(LOG4CPLUS_TSTRING_TO_STRING(tmp).c_str());
    }

    init(filename_, (append_ ? std::ios::app : std::ios::trunc));
}



void
FileAppender::init(const tstring& filename_, 
                   LOG4CPLUS_OPEN_MODE_TYPE mode)
{
    this->filename = filename_;
    open(mode);

    if (bufferSize != 0)
    {
        delete[] buffer;
        buffer = new tchar[bufferSize];
        out.rdbuf ()->pubsetbuf (buffer, bufferSize);
    }

    if(!out.good()) {
        getErrorHandler()->error(  LOG4CPLUS_TEXT("Unable to open file: ") 
                                 + filename);
        return;
    }
    getLogLog().debug(LOG4CPLUS_TEXT("Just opened file: ") + filename);
}



FileAppender::~FileAppender()
{
    destructorImpl();
}



///////////////////////////////////////////////////////////////////////////////
// FileAppender public methods
///////////////////////////////////////////////////////////////////////////////

void 
FileAppender::close()
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex )
        out.close();
        delete[] buffer;
        buffer = 0;
        closed = true;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
}



///////////////////////////////////////////////////////////////////////////////
// FileAppender protected methods
///////////////////////////////////////////////////////////////////////////////

// This method does not need to be locked since it is called by
// doAppend() which performs the locking
void
FileAppender::append(const spi::InternalLoggingEvent& event)
{
    if(!out.good()) {
        if(!reopen()) {
            getErrorHandler()->error(  LOG4CPLUS_TEXT("file is not open: ") 
                                     + filename);
            return;
        }
        // Resets the error handler to make it 
        // ready to handle a future append error.
        else
            getErrorHandler()->reset();
    }

    layout->formatAndAppend(out, event);
    if(immediateFlush) {
        out.flush();
    }
}

void
FileAppender::open(std::ios::openmode mode)
{
    out.open(LOG4CPLUS_TSTRING_TO_STRING(filename).c_str(), mode);
}

bool
FileAppender::reopen()
{
    // When append never failed and the file re-open attempt must 
    // be delayed, set the time when reopen should take place.
    if (reopen_time == log4cplus::helpers::Time () && reopenDelay != 0)
        reopen_time = log4cplus::helpers::Time::gettimeofday()
			+ log4cplus::helpers::Time(reopenDelay);
    else
	{
        // Otherwise, check for end of the delay (or absence of delay) to re-open the file.
        if (reopen_time <= log4cplus::helpers::Time::gettimeofday()
			|| reopenDelay == 0)
		{
            // Close the current file
            out.close();
            out.clear(); // reset flags since the C++ standard specified that all the
                         // flags should remain unchanged on a close

            // Re-open the file.
            open(std::ios::app);

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
// RollingFileAppender ctors and dtor
///////////////////////////////////////////////////////////////////////////////

RollingFileAppender::RollingFileAppender(const tstring& filename_,
    long maxFileSize_, int maxBackupIndex_, bool immediateFlush_)
    : FileAppender(filename_, std::ios::app, immediateFlush_)
{
    init(maxFileSize_, maxBackupIndex_);
}


RollingFileAppender::RollingFileAppender(const Properties& properties)
    : FileAppender(properties, std::ios::app)
{
    int maxFileSize_ = 10*1024*1024;
    int maxBackupIndex_ = 1;
    if(properties.exists( LOG4CPLUS_TEXT("MaxFileSize") )) {
        tstring tmp = properties.getProperty( LOG4CPLUS_TEXT("MaxFileSize") );
        tmp = helpers::toUpper(tmp);
        maxFileSize_ = std::atoi(LOG4CPLUS_TSTRING_TO_STRING(tmp).c_str());
        if(tmp.find( LOG4CPLUS_TEXT("MB") ) == (tmp.length() - 2)) {
            maxFileSize_ *= (1024 * 1024); // convert to megabytes
        }
        if(tmp.find( LOG4CPLUS_TEXT("KB") ) == (tmp.length() - 2)) {
            maxFileSize_ *= 1024; // convert to kilobytes
        }
    }

    if(properties.exists( LOG4CPLUS_TEXT("MaxBackupIndex") )) {
        tstring tmp = properties.getProperty(LOG4CPLUS_TEXT("MaxBackupIndex"));
        maxBackupIndex_ = std::atoi(LOG4CPLUS_TSTRING_TO_STRING(tmp).c_str());
    }

    init(maxFileSize_, maxBackupIndex_);
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
        getLogLog ().warn (oss.str ());
        maxFileSize_ = MINIMUM_ROLLING_LOG_SIZE;
    }

    this->maxFileSize = maxFileSize_;
    this->maxBackupIndex = (std::max)(maxBackupIndex_, 1);
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
    if(!out.good()) {
        if(!reopen()) {
            getErrorHandler()->error(  LOG4CPLUS_TEXT("file is not open: ") 
                                     + filename);
            return;
        }
        // Resets the error handler to make it 
        // ready to handle a future append error.
        else
            getErrorHandler()->reset();
    }

    layout->formatAndAppend(out, event);
    if(immediateFlush) {
        out.flush();
    }
        
    if(out.tellp() > maxFileSize) {
        rollover();
    }
}


void 
RollingFileAppender::rollover()
{
    helpers::LogLog & loglog = getLogLog();

    // Close the current file
    out.close();
    out.clear(); // reset flags since the C++ standard specified that all the
                 // flags should remain unchanged on a close

    // If maxBackups <= 0, then there is no file renaming to be done.
    if (maxBackupIndex > 0)
    {
        rolloverFiles(filename, maxBackupIndex);

        // Rename fileName to fileName.1
        tstring target = filename + LOG4CPLUS_TEXT(".1");

        long ret;

#if defined (WIN32)
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
    bool immediateFlush_, int maxBackupIndex_)
    : FileAppender(filename_, std::ios::app, immediateFlush_)
    , maxBackupIndex(maxBackupIndex_)
{
    init(schedule_);
}



DailyRollingFileAppender::DailyRollingFileAppender(
    const Properties& properties)
    : FileAppender(properties, std::ios::app)
    , maxBackupIndex(10)
{
    DailyRollingFileSchedule theSchedule = DAILY;
    tstring scheduleStr = properties.getProperty(LOG4CPLUS_TEXT("Schedule"));
    scheduleStr = helpers::toUpper(scheduleStr);

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
        getLogLog().warn(  LOG4CPLUS_TEXT("DailyRollingFileAppender::ctor()- \"Schedule\" not valid: ")
                         + properties.getProperty(LOG4CPLUS_TEXT("Schedule")));
        theSchedule = DAILY;
    }
    
    if(properties.exists( LOG4CPLUS_TEXT("MaxBackupIndex") )) {
        tstring tmp = properties.getProperty(LOG4CPLUS_TEXT("MaxBackupIndex"));
        maxBackupIndex = std::atoi(LOG4CPLUS_TSTRING_TO_STRING(tmp).c_str());
    }

    init(theSchedule);
}



void
DailyRollingFileAppender::init(DailyRollingFileSchedule schedule_)
{
    this->schedule = schedule_;

    Time now = Time::gettimeofday();
    now.usec(0);
    struct tm time;
    now.localtime(&time);

    time.tm_sec = 0;
    switch (schedule)
    {
    case MONTHLY:
        time.tm_mday = 1;
        time.tm_hour = 0;
        time.tm_min = 0;
        break;

    case WEEKLY:
        time.tm_mday -= (time.tm_wday % 7);
        time.tm_hour = 0;
        time.tm_min = 0;
        break;

    case DAILY:
        time.tm_hour = 0;
        time.tm_min = 0;
        break;

    case TWICE_DAILY:
        if(time.tm_hour >= 12) {
            time.tm_hour = 12;
        }
        else {
            time.tm_hour = 0;
        }
        time.tm_min = 0;
        break;

    case HOURLY:
        time.tm_min = 0;
        break;

    case MINUTELY:
        break;
    };
    now.setTime(&time);

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
    if(!out.good()) {
        if(!reopen()) {
            getErrorHandler()->error(  LOG4CPLUS_TEXT("file is not open: ") 
                                     + filename);
            return;
        }
        // Resets the error handler to make it 
        // ready to handle a future append error.
        else
            getErrorHandler()->reset();
    }

    if(event.getTimestamp() >= nextRolloverTime) {
        rollover();
    }

    layout->formatAndAppend(out, event);
    if(immediateFlush) {
        out.flush();
    }
}



void
DailyRollingFileAppender::rollover()
{
    // Close the current file
    out.close();
    out.clear(); // reset flags since the C++ standard specified that all the
                 // flags should remain unchanged on a close

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

    helpers::LogLog & loglog = getLogLog();
    long ret;

#if defined (WIN32)
    // Try to remove the target first. It seems it is not
    // possible to rename over existing file, e.g. "log.2009-11-07.1".
    ret = file_remove (backupTarget);
#endif

    // Rename e.g. "log.2009-11-07" to "log.2009-11-07.1".
    ret = file_rename (scheduledFilename, backupTarget);
    loglog_renaming_result (loglog, scheduledFilename, backupTarget, ret);

#if defined (WIN32)
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
    log4cplus::helpers::Time now = Time::gettimeofday();
    if (now >= nextRolloverTime)
    {
        scheduledFilename = getFilename(now);
        nextRolloverTime = calculateNextRolloverTime(now);
    }
}



Time
DailyRollingFileAppender::calculateNextRolloverTime(const Time& t) const
{
    switch(schedule)
    {
    case MONTHLY: 
    {
        struct tm nextMonthTime;
        t.localtime(&nextMonthTime);
        nextMonthTime.tm_mon += 1;
        nextMonthTime.tm_isdst = 0;

        Time ret;
        if(ret.setTime(&nextMonthTime) == -1) {
            getLogLog().error(
                LOG4CPLUS_TEXT("DailyRollingFileAppender::calculateNextRolloverTime()-")
                LOG4CPLUS_TEXT(" setTime() returned error"));
            // Set next rollover to 31 days in future.
            ret = (t + Time(2678400));
        }

        return ret;
    }

    case WEEKLY:
        return (t + Time(7 * 24 * 60 * 60));

    default:
        getLogLog ().error (
            LOG4CPLUS_TEXT ("DailyRollingFileAppender::calculateNextRolloverTime()-")
            LOG4CPLUS_TEXT (" invalid schedule value"));
        // Fall through.

    case DAILY:
        return (t + Time(24 * 60 * 60));

    case TWICE_DAILY:
        return (t + Time(12 * 60 * 60));

    case HOURLY:
        return (t + Time(60 * 60));

    case MINUTELY:
        return (t + Time(60));
    };
}



tstring
DailyRollingFileAppender::getFilename(const Time& t) const
{
    tchar const * pattern = 0;
    switch (schedule)
    {
    case MONTHLY:
        pattern = LOG4CPLUS_TEXT("%Y-%m");
        break;

    case WEEKLY:
        pattern = LOG4CPLUS_TEXT("%Y-%W");
        break;

    default:
        getLogLog ().error (
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

    tstring result (filename);
    result += LOG4CPLUS_TEXT(".");
    result += t.getFormattedTime(pattern, false);
    return result;
}

} // namespace log4cplus
