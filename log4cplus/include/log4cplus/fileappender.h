// Module:  Log4CPLUS
// File:    fileappender.h
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

/** @file */

#ifndef _LOG4CPLUS_FILE_APPENDER_HEADER_
#define _LOG4CPLUS_FILE_APPENDER_HEADER_

#include <log4cplus/config.h>
#include <log4cplus/appender.h>
#include <log4cplus/fstreams.h>
#include <log4cplus/helpers/property.h>


namespace log4cplus {

    /**
     * Appends log events to a file. 
     */
    class LOG4CPLUS_EXPORT FileAppender : public Appender {
    public:
      // Ctors
        FileAppender(const log4cplus::tstring& filename, 
                     int mode = LOG4CPLUS_FSTREAM_NAMESPACE::ios::trunc);
        FileAppender(const log4cplus::helpers::Properties& properties,
                     int mode = LOG4CPLUS_FSTREAM_NAMESPACE::ios::trunc);

      // Dtor
        virtual ~FileAppender();

      // Methods
        virtual void close();

    protected:
        virtual void append(const spi::InternalLoggingEvent& event);

      // Data
        log4cplus::tofstream out;
        log4cplus::tstring filename;

    private:
        void init(const log4cplus::tstring& filename, int mode);

      // Disallow copying of instances of this class
        FileAppender(const FileAppender&);
        FileAppender& operator=(const FileAppender&);
    };



    /**
     * RollingFileAppender extends FileAppender to backup the log files when 
     * they reach a certain size. 
     */
    class LOG4CPLUS_EXPORT RollingFileAppender : public FileAppender {
    public:
      // Ctors
        RollingFileAppender(const log4cplus::tstring& filename,
                            long maxFileSize = 10*1024*1024, // 10 MB
                            int maxBackupIndex = 1);
        RollingFileAppender(const log4cplus::helpers::Properties& properties);

      // Dtor
        virtual ~RollingFileAppender();

    protected:
        virtual void append(const spi::InternalLoggingEvent& event);
        void rollover();

      // Data
        long maxFileSize;
        int maxBackupIndex;

    private:
        void init(long maxFileSize, int maxBackupIndex);
    };



    enum DailyRollingFileSchedule { MONTHLY, WEEKLY, DAILY,
                                    TWICE_DAILY, HOURLY, MINUTELY};

    /**
     * DailyRollingFileAppender extends {@link FileAppender} so that the
     * underlying file is rolled over at a user chosen frequency.
     *
     * <p>
     *         
     */
    class LOG4CPLUS_EXPORT DailyRollingFileAppender : public FileAppender {
    public:
      // Ctors
        DailyRollingFileAppender(const log4cplus::tstring& filename,
                                 DailyRollingFileSchedule schedule = DAILY);
        DailyRollingFileAppender(const log4cplus::helpers::Properties& properties);

      // Dtor
        virtual ~DailyRollingFileAppender();

    protected:
        virtual void append(const spi::InternalLoggingEvent& event);
        void rollover();
        log4cplus::helpers::Time calculateNextRolloverTime(const log4cplus::helpers::Time& t) const;
        log4cplus::tstring getFilename(const log4cplus::helpers::Time& t) const;

      // Data
        DailyRollingFileSchedule schedule;
        log4cplus::tstring scheduledFilename;
        log4cplus::helpers::Time nextRolloverTime;

    private:
        void init(DailyRollingFileSchedule schedule);
    };

} // end namespace log4cplus

#endif // _LOG4CPLUS_FILE_APPENDER_HEADER_

