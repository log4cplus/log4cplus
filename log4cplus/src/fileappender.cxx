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

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;

#define MINIMUM_ROLLING_LOG_SIZE 200*1024L


///////////////////////////////////////////////////////////////////////////////
// File LOCAL definitions
///////////////////////////////////////////////////////////////////////////////

namespace {

    static
    void rolloverFiles(const log4cplus::tstring& filename, unsigned int maxBackupIndex)
    {
        SharedObjectPtr<LogLog> loglog = LogLog::getLogLog();

        // Delete the oldest file
        log4cplus::tostringstream buffer;
        buffer << filename << LOG4CPLUS_TEXT(".") << maxBackupIndex;
        remove(LOG4CPLUS_TSTRING_TO_STRING(buffer.str()).c_str());

        // Map {(maxBackupIndex - 1), ..., 2, 1} to {maxBackupIndex, ..., 3, 2}
        for(int i=maxBackupIndex - 1; i >= 1; i--) {
            log4cplus::tostringstream source;
            log4cplus::tostringstream target;

            source << filename << LOG4CPLUS_TEXT(".") << i;
            target << filename << LOG4CPLUS_TEXT(".") << (i+1);
            if(rename(LOG4CPLUS_TSTRING_TO_STRING(source.str()).c_str(), 
                      LOG4CPLUS_TSTRING_TO_STRING(target.str()).c_str()) == 0) 
            {
                loglog->debug(  LOG4CPLUS_TEXT("Renamed file ") 
                              + source.str() 
                              + LOG4CPLUS_TEXT(" to ")
                              + target.str());
            }
        }
    } // end rolloverFiles()
    
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::FileAppender ctors and dtor
///////////////////////////////////////////////////////////////////////////////

log4cplus::FileAppender::FileAppender(const log4cplus::tstring& filename, 
                                      LOG4CPLUS_OPEN_MODE_TYPE mode,
                                      bool immediateFlush)
: immediateFlush(immediateFlush)
{
    init(filename, mode);
}


log4cplus::FileAppender::FileAppender(const Properties& properties, 
                                      LOG4CPLUS_OPEN_MODE_TYPE mode)
: Appender(properties),
  immediateFlush(true)
{
    bool append = (mode == ios::app);
    tstring filename = properties.getProperty( LOG4CPLUS_TEXT("File") );
    if(filename.length() == 0) {
        getErrorHandler()->error( LOG4CPLUS_TEXT("Invalid filename") );
        return;
    }
    if(properties.exists( LOG4CPLUS_TEXT("ImmediateFlush") )) {
        tstring tmp = properties.getProperty( LOG4CPLUS_TEXT("ImmediateFlush") );
        immediateFlush = (toLower(tmp) == LOG4CPLUS_TEXT("true"));
    }
    if(properties.exists( LOG4CPLUS_TEXT("Append") )) {
        tstring tmp = properties.getProperty( LOG4CPLUS_TEXT("Append") );
        append = (toLower(tmp) == LOG4CPLUS_TEXT("true"));
    }

    init(filename, (append ? ios::app : ios::trunc));
}



void
log4cplus::FileAppender::init(const log4cplus::tstring& filename, 
                              LOG4CPLUS_OPEN_MODE_TYPE mode)
{
    this->filename = filename;
    out.open(LOG4CPLUS_TSTRING_TO_STRING(filename).c_str(), mode);

    if(!out.good()) {
        getErrorHandler()->error(  LOG4CPLUS_TEXT("Unable to open file: ") 
                                 + filename);
        return;
    }
    getLogLog().debug(LOG4CPLUS_TEXT("Just opened file: ") + filename);
}



log4cplus::FileAppender::~FileAppender()
{
    destructorImpl();
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::FileAppender public methods
///////////////////////////////////////////////////////////////////////////////

void 
log4cplus::FileAppender::close()
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex )
        out.close();
        closed = true;
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX;
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::FileAppender protected methods
///////////////////////////////////////////////////////////////////////////////

// This method does not need to be locked since it is called by
// doAppend() which performs the locking
void
log4cplus::FileAppender::append(const spi::InternalLoggingEvent& event)
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
// log4cplus::RollingFileAppender ctors and dtor
///////////////////////////////////////////////////////////////////////////////

log4cplus::RollingFileAppender::RollingFileAppender(const log4cplus::tstring& filename,
                                                    long maxFileSize,
                                                    int maxBackupIndex,
                                                    bool immediateFlush)
: FileAppender(filename, ios::app, immediateFlush)
{
    init(maxFileSize, maxBackupIndex);
}



log4cplus::RollingFileAppender::RollingFileAppender(const Properties& properties)
: FileAppender(properties, ios::app)
{
    int maxFileSize = 10*1024*1024;
    int maxBackupIndex = 1;
    if(properties.exists( LOG4CPLUS_TEXT("MaxFileSize") )) {
        tstring tmp = properties.getProperty( LOG4CPLUS_TEXT("MaxFileSize") );
        tmp = toUpper(tmp);
        maxFileSize = atoi(LOG4CPLUS_TSTRING_TO_STRING(tmp).c_str());
        if(tmp.find( LOG4CPLUS_TEXT("MB") ) == (tmp.length() - 2)) {
            maxFileSize *= (1024 * 1024); // convert to megabytes
        }
        if(tmp.find( LOG4CPLUS_TEXT("KB") ) == (tmp.length() - 2)) {
            maxFileSize *= 1024; // convert to kilobytes
        }
    }

    if(properties.exists( LOG4CPLUS_TEXT("MaxBackupIndex") )) {
        tstring tmp = properties.getProperty(LOG4CPLUS_TEXT("MaxBackupIndex"));
        maxBackupIndex = atoi(LOG4CPLUS_TSTRING_TO_STRING(tmp).c_str());
    }

    init(maxFileSize, maxBackupIndex);
}



void
log4cplus::RollingFileAppender::init(long maxFileSize, int maxBackupIndex)
{
    this->maxFileSize = max(maxFileSize, MINIMUM_ROLLING_LOG_SIZE);
    this->maxBackupIndex = max(maxBackupIndex, 1);
}



log4cplus::RollingFileAppender::~RollingFileAppender()
{
    destructorImpl();
}




///////////////////////////////////////////////////////////////////////////////
// log4cplus::RollingFileAppender protected methods
///////////////////////////////////////////////////////////////////////////////

// This method does not need to be locked since it is called by
// doAppend() which performs the locking
void
log4cplus::RollingFileAppender::append(const spi::InternalLoggingEvent& event)
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
log4cplus::RollingFileAppender::rollover()
{
    // If maxBackups <= 0, then there is no file renaming to be done.
    if(maxBackupIndex > 0) {
        rolloverFiles(filename, maxBackupIndex);

        // Close the current file
        out.close();
        out.clear(); // reset flags since the C++ standard specified that all the
                     // flags should remain unchanged on a close

        // Rename fileName to fileName.1
        log4cplus::tstring target = filename + LOG4CPLUS_TEXT(".1");
        getLogLog().debug(  LOG4CPLUS_TEXT("Renaming file ") 
                          + filename 
                          + LOG4CPLUS_TEXT(" to ")
                          + target);
        rename(LOG4CPLUS_TSTRING_TO_STRING(filename).c_str(), 
               LOG4CPLUS_TSTRING_TO_STRING(target).c_str());

        // Open a new file
        out.open(LOG4CPLUS_TSTRING_TO_STRING(filename).c_str(), 
                 ios::out | ios::trunc);
    }
    else {
        getLogLog().debug(  filename 
                          + LOG4CPLUS_TEXT(" has no backups specified"));
        // Close the current file
        out.close();
        out.clear(); // reset flags since the C++ standard specified that all the
                     // flags should remain unchanged on a close

        // Open it up again in truncation mode
        out.open(LOG4CPLUS_TSTRING_TO_STRING(filename).c_str(), 
                 ios::out | ios::trunc);
    }
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::DailyRollingFileAppender ctors and dtor
///////////////////////////////////////////////////////////////////////////////

DailyRollingFileAppender::DailyRollingFileAppender(const log4cplus::tstring& filename,
                                                   DailyRollingFileSchedule schedule,
                                                   bool immediateFlush,
                                                   int maxBackupIndex)
: FileAppender(filename, ios::app, immediateFlush),
  maxBackupIndex(maxBackupIndex)
{
    init(schedule);
}



DailyRollingFileAppender::DailyRollingFileAppender(const Properties& properties)
: FileAppender(properties, ios::app),
  maxBackupIndex(10)
{
    DailyRollingFileSchedule theSchedule = DAILY;
    tstring scheduleStr = properties.getProperty(LOG4CPLUS_TEXT("Schedule"));\
    scheduleStr = toUpper(scheduleStr);

    if(scheduleStr == LOG4CPLUS_TEXT("MONTHLY")) {
        theSchedule = MONTHLY;
    }
    else if(scheduleStr == LOG4CPLUS_TEXT("WEEKLY")) {
        theSchedule = WEEKLY;
    }
    else if(scheduleStr == LOG4CPLUS_TEXT("DAILY")) {
        theSchedule = DAILY;
    }
    else if(scheduleStr == LOG4CPLUS_TEXT("TWICE_DAILY")) {
        theSchedule = TWICE_DAILY;
    }
    else if(scheduleStr == LOG4CPLUS_TEXT("HOURLY")) {
        theSchedule = HOURLY;
    }
    else if(scheduleStr == LOG4CPLUS_TEXT("MINUTELY")) {
        theSchedule = MINUTELY;
    }
    else {
        getLogLog().warn(  LOG4CPLUS_TEXT("DailyRollingFileAppender::ctor()- \"Schedule\" not valid: ")
                         + properties.getProperty(LOG4CPLUS_TEXT("Schedule")));
    }
    
    if(properties.exists( LOG4CPLUS_TEXT("MaxBackupIndex") )) {
        tstring tmp = properties.getProperty(LOG4CPLUS_TEXT("MaxBackupIndex"));
        maxBackupIndex = atoi(LOG4CPLUS_TSTRING_TO_STRING(tmp).c_str());
    }

    init(theSchedule);
}



void
DailyRollingFileAppender::init(DailyRollingFileSchedule schedule)
{
    this->schedule = schedule;

    Time now = Time::gettimeofday();
    now.usec(0);
    struct tm time;
    now.localtime(&time);

    time.tm_sec = 0;
    switch(schedule) {
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
// log4cplus::DailyRollingFileAppender public methods
///////////////////////////////////////////////////////////////////////////////

void
DailyRollingFileAppender::close()
{
    rollover();
    FileAppender::close();
}



///////////////////////////////////////////////////////////////////////////////
// log4cplus::DailyRollingFileAppender protected methods
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
    log4cplus::tostringstream backupTarget;
    backupTarget << scheduledFilename << LOG4CPLUS_TEXT(".") << 1;
    if( rename(LOG4CPLUS_TSTRING_TO_STRING(scheduledFilename).c_str(), 
               LOG4CPLUS_TSTRING_TO_STRING(backupTarget.str()).c_str()) == 0 )
    {
        getLogLog().debug(  LOG4CPLUS_TEXT("Renamed file ") 
                          + scheduledFilename 
                          + LOG4CPLUS_TEXT(" to ")
                          + backupTarget.str());
    }
    
    // Rename filename to scheduledFilename
    getLogLog().debug(  LOG4CPLUS_TEXT("Renaming file ") + filename 
                      + LOG4CPLUS_TEXT(" to ") + scheduledFilename);
    rename(LOG4CPLUS_TSTRING_TO_STRING(filename).c_str(), 
           LOG4CPLUS_TSTRING_TO_STRING(scheduledFilename).c_str());

    // Open a new file
    out.open(LOG4CPLUS_TSTRING_TO_STRING(filename).c_str(), 
             ios::out | ios::trunc);

    // Calculate the next rollover time
    if(Time::gettimeofday() >= nextRolloverTime) {
        scheduledFilename = getFilename(nextRolloverTime);
        nextRolloverTime = calculateNextRolloverTime(nextRolloverTime);
    }
}



log4cplus::helpers::Time
DailyRollingFileAppender::calculateNextRolloverTime(const Time& t) const
{
    switch(schedule) {
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

    case DAILY:
        return (t + Time(86400)); //      24 * 60 * 60 seconds

    case TWICE_DAILY:
        return (t + Time(43200)); //      12 * 60 * 60 seconds

    case HOURLY:
        return (t + Time(3600));  //           60 * 60 seconds

    case MINUTELY:
        return (t + Time(60));    //                60 seconds
    };

    getLogLog().error(LOG4CPLUS_TEXT("DailyRollingFileAppender::calculateNextRolloverTime()- invalid schedule value"));
    return (t + Time(86400));
}



log4cplus::tstring
DailyRollingFileAppender::getFilename(const log4cplus::helpers::Time& t) const
{
    tstring pattern;
    switch(schedule) {
    case MONTHLY:
        pattern = LOG4CPLUS_TEXT("%Y-%m");
        break;

    case WEEKLY:
        pattern = LOG4CPLUS_TEXT("%Y-%W");
        break;

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

