// Module:  Log4CPLUS
// File:    fileappender.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/streams.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/helpers/timehelper.h>
#include <log4cplus/spi/loggingevent.h>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>


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

static 
int
file_rename (tstring const & src, tstring const & target)
{
#if defined (UNICODE) && defined (WIN32)
    return _wrename (src.c_str (), target.c_str ()) == 0 ? 0 : -1;
#else
    return rename (LOG4CPLUS_TSTRING_TO_STRING (src).c_str (),
        LOG4CPLUS_TSTRING_TO_STRING (target).c_str ()) == 0 ? 0 : -1;
#endif
}


static
int
file_remove (tstring const & src)
{
#if defined (UNICODE) && defined (WIN32)
    return _wremove (src.c_str ()) == 0 ? 0 : -1;
#else
    return remove (LOG4CPLUS_TSTRING_TO_STRING (src).c_str ()) == 0
        ? 0 : -1;
#endif
}


static
void
loglog_renaming_result (helpers::LogLog & loglog, tstring const & src,
    tstring const & target, int ret)
{
    if (ret == 0)
    {
        loglog.debug (
            LOG4CPLUS_TEXT("Renamed file ") 
            + src 
            + LOG4CPLUS_TEXT(" to ")
            + target);
    }
    else
    {
        loglog.error (
            LOG4CPLUS_TEXT("Failed to rename file from ") 
            + target 
            + LOG4CPLUS_TEXT(" to ")
            + target);
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
    tstring buffer_str = buffer.str ();
    int ret = file_remove (buffer.str ());

    // Map {(maxBackupIndex - 1), ..., 2, 1} to {maxBackupIndex, ..., 3, 2}
    for (int i = maxBackupIndex - 1; i >= 1; --i)
    {
        tostringstream source_oss;
        tostringstream target_oss;

        source_oss << filename << LOG4CPLUS_TEXT(".") << i;
        target_oss << filename << LOG4CPLUS_TEXT(".") << (i+1);

        tstring source (source_oss.str ());
        tstring target (target_oss.str ());

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

FileAppender::FileAppender(const tstring& _filename, 
						   LOG4CPLUS_OPEN_MODE_TYPE mode, 
						   bool _immediateFlush)
    : immediateFlush(_immediateFlush)
{
    init(_filename, mode);
}


FileAppender::FileAppender(const Properties& properties, 
                           LOG4CPLUS_OPEN_MODE_TYPE mode)
    : Appender(properties), immediateFlush(true)
{
    const tstring& filenameProperty = properties.getProperty( LOG4CPLUS_TEXT("File") );
    if(filenameProperty.empty())
	{
        getErrorHandler()->error( LOG4CPLUS_TEXT("Invalid filename") );
    }
	else
	{
		bool mustAppend = (mode == std::ios::app);
		if(properties.exists( LOG4CPLUS_TEXT("ImmediateFlush") )) {
			tstring const & tmp = properties.getProperty( LOG4CPLUS_TEXT("ImmediateFlush") );
			immediateFlush = (helpers::toLower(tmp) == LOG4CPLUS_TEXT("true"));
		}
		if(properties.exists( LOG4CPLUS_TEXT("Append") )) {
			tstring const & tmp = properties.getProperty( LOG4CPLUS_TEXT("Append") );
			mustAppend = (helpers::toLower(tmp) == LOG4CPLUS_TEXT("true"));
		}
		
		init(filenameProperty, (mustAppend ? std::ios::app : std::ios::trunc));
	}
}



void
FileAppender::init(const tstring& _filename, 
                   LOG4CPLUS_OPEN_MODE_TYPE mode)
{
    this->filename = _filename;
    out.open(LOG4CPLUS_TSTRING_TO_STRING(_filename).c_str(), mode);

    if(out.good()) 
	{
		getLogLog().debug(LOG4CPLUS_TEXT("Just opened file: ") + _filename);
	}
    else
	{
		getErrorHandler()->error(LOG4CPLUS_TEXT("Unable to open file: ") 
								 + _filename);
    }
    
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
        getErrorHandler()->error(  LOG4CPLUS_TEXT("file is not open: ") 
                                 + filename);
        return;
    }

    layout->formatAndAppend(out, event);
    if(immediateFlush) {
        out.flush();
    }
}


///////////////////////////////////////////////////////////////////////////////
// RollingFileAppender ctors and dtor
///////////////////////////////////////////////////////////////////////////////

RollingFileAppender::RollingFileAppender(const tstring& _filename,
										 long _maxFileSize, 
										 int _maxBackupIndex, 
										 bool _immediateFlush)
    : FileAppender(_filename, std::ios::app, _immediateFlush)
{
    init(_maxFileSize, _maxBackupIndex);
}


RollingFileAppender::RollingFileAppender(const Properties& properties)
    : FileAppender(properties, std::ios::app)
{
    long maxSize = 10*1024*1024;
    tstring tmp (
        helpers::toUpper (
            properties.getProperty (LOG4CPLUS_TEXT ("MaxFileSize"))));
    if (! tmp.empty ())
    {
        maxSize = atoi(LOG4CPLUS_TSTRING_TO_STRING(tmp).c_str());
        tstring::size_type const len = tmp.length();
        if (len > 2
            && tmp.compare (len - 2, 2, LOG4CPLUS_TEXT("MB")) == 0)
            maxSize *= (1024 * 1024); // convert to megabytes
        else if (len > 2
            && tmp.compare (len - 2, 2, LOG4CPLUS_TEXT("KB")) == 0)
            maxSize *= 1024; // convert to kilobytes
    }

    tmp = properties.getProperty (LOG4CPLUS_TEXT("MaxBackupIndex"));

    const int maxIndex = (tmp.empty() ? 1 : atoi(LOG4CPLUS_TSTRING_TO_STRING(tmp).c_str()));

    init(maxSize, maxIndex);
}


void
RollingFileAppender::init(long _maxFileSize, int _maxBackupIndex)
{
    this->maxFileSize = std::max(_maxFileSize, MINIMUM_ROLLING_LOG_SIZE);
    this->maxBackupIndex = std::max(_maxBackupIndex, 1);
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
        getErrorHandler()->error(  LOG4CPLUS_TEXT("file is not open: ") 
                                 + filename);
        return;
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

        int ret;

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
    out.open(LOG4CPLUS_TSTRING_TO_STRING(filename).c_str(), 
        std::ios::out | std::ios::trunc);
    loglog_opening_result (loglog, out, filename);
}


///////////////////////////////////////////////////////////////////////////////
// DailyRollingFileAppender ctors and dtor
///////////////////////////////////////////////////////////////////////////////

DailyRollingFileAppender::DailyRollingFileAppender(const tstring& _filename, 
												   DailyRollingFileSchedule _schedule,
												   bool _immediateFlush, 
												   int _maxBackupIndex)
    : FileAppender(_filename, std::ios::app, _immediateFlush)
    , maxBackupIndex(_maxBackupIndex)
{
    init(_schedule);
}



DailyRollingFileAppender::DailyRollingFileAppender(
    const Properties& properties)
    : FileAppender(properties, std::ios::app)
    , maxBackupIndex(10)
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
        getLogLog().warn(  LOG4CPLUS_TEXT("DailyRollingFileAppender::ctor()- \"Schedule\" not valid: ")
                         + properties.getProperty(LOG4CPLUS_TEXT("Schedule")));
        theSchedule = DAILY;
    }
    
    tstring const tmp (properties.getProperty (
        LOG4CPLUS_TEXT("MaxBackupIndex"), LOG4CPLUS_TEXT("10")));
    maxBackupIndex = atoi(LOG4CPLUS_TSTRING_TO_STRING(tmp).c_str());

    init(theSchedule);
}



void
DailyRollingFileAppender::init(DailyRollingFileSchedule sch)
{
    this->schedule = sch;

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
        getErrorHandler()->error(  LOG4CPLUS_TEXT("file is not open: ") 
                                 + filename);
        return;
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
    rolloverFiles(scheduledFilename, maxBackupIndex);
    tostringstream backup_target_oss;
    backup_target_oss << scheduledFilename << LOG4CPLUS_TEXT(".") << 1;
    tstring backupTarget = backup_target_oss.str();

    helpers::LogLog & loglog = getLogLog();
    int ret;

#if defined (WIN32)
    // Try to remove the target first. It seems it is not
    // possible to rename over existing file.
    ret = file_remove (backupTarget);
#endif

    ret = file_rename (scheduledFilename, backupTarget);
    loglog_renaming_result (loglog, scheduledFilename, backupTarget, ret);
    
    // Rename filename to scheduledFilename
    loglog.debug(
        LOG4CPLUS_TEXT("Renaming file ")
        + filename 
        + LOG4CPLUS_TEXT(" to ")
        + scheduledFilename);
    ret = file_rename (filename, scheduledFilename);
    loglog_renaming_result (loglog, filename, scheduledFilename, ret);

    // Open a new file
    out.open(LOG4CPLUS_TSTRING_TO_STRING(filename).c_str(), 
        std::ios::out | std::ios::trunc);
    loglog_opening_result (loglog, out, filename);

    // Calculate the next rollover time
    if (Time::gettimeofday() >= nextRolloverTime)
    {
        scheduledFilename = getFilename(nextRolloverTime);
        nextRolloverTime = calculateNextRolloverTime(nextRolloverTime);
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
            getLogLog().error(LOG4CPLUS_TEXT("DailyRollingFileAppender::calculateNextRolloverTime()- setTime() returned error"));
            ret = (t + Time(2678400));
        }

        return ret;
    }

    case WEEKLY:
        return (t + Time(604800)); // 7 * 24 * 60 * 60 seconds

    default:
        getLogLog ().error (
            LOG4CPLUS_TEXT ("DailyRollingFileAppender::calculateNextRolloverTime()-")
            LOG4CPLUS_TEXT (" invalid schedule value"));
        // Fall through.

    case DAILY:
        return (t + Time(86400)); //      24 * 60 * 60 seconds

    case TWICE_DAILY:
        return (t + Time(43200)); //      12 * 60 * 60 seconds

    case HOURLY:
        return (t + Time(3600));  //           60 * 60 seconds

    case MINUTELY:
        return (t + Time(60));    //                60 seconds
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

    return filename + LOG4CPLUS_TEXT(".") + t.getFormattedTime(pattern, false);
}

} // namespace log4cplus
