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
// $Log: not supported by cvs2svn $
// Revision 1.15  2003/06/26 21:34:02  baldheadedguy
// Added LOG4CPLUS_TEXT to strings in DailyRollingFileAppender::getFilename and
// DailyRollingFileAppender constructor.  In both cases, the comiple was failing
// under UNICODE.
//
// Revision 1.14  2003/06/23 23:14:12  tcsmith
// Corrected the DailyRollingFileAppender's weekly and montly calculations.
//
// Revision 1.13  2003/06/23 20:56:43  tcsmith
// Modified to support the changes in the spi::InternalLoggingEvent class.
//
// Revision 1.12  2003/06/13 20:56:24  tcsmith
// Made changes to make the DEC cXX 6.1 compiler happy.
//
// Revision 1.11  2003/06/12 23:55:49  tcsmith
// Initial implementation of the DailyRollingFileAppender class.
//
// Revision 1.10  2003/06/06 17:04:31  tcsmith
// Changed the ctor to take a 'const' Properties object.
//
// Revision 1.9  2003/06/03 20:19:42  tcsmith
// Modified the close() method to set "closed = true;".
//
// Revision 1.8  2003/04/19 23:04:31  tcsmith
// Fixed UNICODE support.
//
// Revision 1.7  2003/04/18 21:56:39  tcsmith
// Converted from std::string to log4cplus::tstring.
//
// Revision 1.6  2003/04/03 00:49:07  tcsmith
// Standardized the formatting.
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
// log4cplus::FileAppender ctors and dtor
///////////////////////////////////////////////////////////////////////////////

log4cplus::FileAppender::FileAppender(const log4cplus::tstring& filename, 
                                      int mode)
{
    init(filename, mode);
}


log4cplus::FileAppender::FileAppender(const Properties& properties, int mode)
: Appender(properties)
{
     tstring filename = properties.getProperty( LOG4CPLUS_TEXT("File") );
     if(filename.length() == 0) {
         getErrorHandler()->error( LOG4CPLUS_TEXT("Invalid filename") );
         return;
     }

     init(filename, mode);
}



void
log4cplus::FileAppender::init(const log4cplus::tstring& filename, int mode)
{
    this->filename = filename;
    out.open(LOG4CPLUS_TSTRING_TO_STRING(filename).c_str(), mode);

    if(!out.good()) {
        getErrorHandler()->error(  LOG4CPLUS_TEXT("Unable to open file: ") 
                                 + filename);
        return;
    }
    getLogLog().debug(LOG4CPLUS_TEXT("Just opened file: ") + filename);

    int columns = 80;
    log4cplus::tstring commentline;
    commentline.resize(columns);
    fill_n(commentline.begin(), columns, LOG4CPLUS_TEXT('#'));
    
    out << commentline << endl
        << LOG4CPLUS_TEXT("#") << endl
        << LOG4CPLUS_TEXT("# Created Appender on: ")
        << Time::gettimeofday().getFormattedTime(LOG4CPLUS_TEXT("%m-%d-%y %H:%M:%S")) 
        << endl
        << LOG4CPLUS_TEXT("#") << endl
        << commentline << endl;
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
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
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
}




///////////////////////////////////////////////////////////////////////////////
// log4cplus::RollingFileAppender ctors and dtor
///////////////////////////////////////////////////////////////////////////////

log4cplus::RollingFileAppender::RollingFileAppender(const log4cplus::tstring& filename,
                                                    long maxFileSize,
                                                    int maxBackupIndex)
: FileAppender(filename, ios::app)
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
        
    if(out.tellp() > maxFileSize) {
        rollover();
    }
}



void 
log4cplus::RollingFileAppender::rollover()
{
    // If maxBackups <= 0, then there is no file renaming to be done.
    if(maxBackupIndex > 0) {
        // Delete the oldest file
        log4cplus::tostringstream buffer;
        buffer << filename << LOG4CPLUS_TEXT('.') << maxBackupIndex;
        remove(LOG4CPLUS_TSTRING_TO_STRING(buffer.str()).c_str());

        // Map {(maxBackupIndex - 1), ..., 2, 1} to {maxBackupIndex, ..., 3, 2}
        for(int i=maxBackupIndex - 1; i >= 1; i--) {
            log4cplus::tostringstream source;
            log4cplus::tostringstream target;

            source << filename << LOG4CPLUS_TEXT('.') << i;
            target << filename << LOG4CPLUS_TEXT('.') << (i+1);
            if(rename(LOG4CPLUS_TSTRING_TO_STRING(source.str()).c_str(), 
                      LOG4CPLUS_TSTRING_TO_STRING(target.str()).c_str()) == 0) 
            {
                getLogLog().debug(  LOG4CPLUS_TEXT("Renamed file ") 
                                  + source.str() 
                                  + LOG4CPLUS_TEXT(" to ")
                                  + target.str());
            }
        }

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
                                                   DailyRollingFileSchedule schedule)
: FileAppender(filename, ios::app)
{
    init(schedule);
}



DailyRollingFileAppender::DailyRollingFileAppender(const Properties& properties)
: FileAppender(properties)
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
}



void
DailyRollingFileAppender::rollover()
{
    // Close the current file
    out.close();
    out.clear(); // reset flags since the C++ standard specified that all the
                 // flags should remain unchanged on a close

    // Rename fileName to fileName.1
    getLogLog().debug(  LOG4CPLUS_TEXT("Renaming file ") + filename 
                      + LOG4CPLUS_TEXT(" to ") + scheduledFilename);
    rename(LOG4CPLUS_TSTRING_TO_STRING(filename).c_str(), 
           LOG4CPLUS_TSTRING_TO_STRING(scheduledFilename).c_str());

    // Open a new file
    out.open(LOG4CPLUS_TSTRING_TO_STRING(filename).c_str(), 
             ios::out | ios::trunc);

    // Calculate the next rollover time
    scheduledFilename = getFilename(nextRolloverTime);
    nextRolloverTime = calculateNextRolloverTime(nextRolloverTime);
}



log4cplus::helpers::Time
DailyRollingFileAppender::calculateNextRolloverTime(const Time& t) const
{
    switch(schedule) {
    case MONTHLY: {
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
        break;

    case WEEKLY:
        return (t + Time(604800)); // 7 * 24 * 60 * 60 seconds
        break;

    case DAILY:
        return (t + Time(86400)); //      24 * 60 * 60 seconds
        break;

    case TWICE_DAILY:
        return (t + Time(43200)); //      12 * 60 * 60 seconds
        break;

    case HOURLY:
        return (t + Time(3600));  //           60 * 60 seconds
        break;

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




