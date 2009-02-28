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

#include <log4cplus/config.hxx>
#include <log4cplus/appender.h>
#include <log4cplus/fstreams.h>
#include <log4cplus/helpers/property.h>

#if defined(__DECCXX)
#   define LOG4CPLUS_OPEN_MODE_TYPE LOG4CPLUS_FSTREAM_NAMESPACE::ios::open_mode
#else
#   define LOG4CPLUS_OPEN_MODE_TYPE LOG4CPLUS_FSTREAM_NAMESPACE::ios::openmode
#endif

namespace log4cplus {

    /**
     * Appends log events to a file. 
     */
    class LOG4CPLUS_EXPORT FileAppender : public Appender {
    public:
      // Ctors
        FileAppender(const log4cplus::tstring& filename, 
                     LOG4CPLUS_OPEN_MODE_TYPE mode = LOG4CPLUS_FSTREAM_NAMESPACE::ios::trunc,
                     bool immediateFlush = true);
        FileAppender(const log4cplus::helpers::Properties& properties,
                     LOG4CPLUS_OPEN_MODE_TYPE mode = LOG4CPLUS_FSTREAM_NAMESPACE::ios::trunc);

      // Dtor
        virtual ~FileAppender();

      // Methods
        virtual void close();

    protected:
        virtual void append(const spi::InternalLoggingEvent& event);

      // Data
        /**
         * Immediate flush means that the underlying writer or output stream
         * will be flushed at the end of each append operation. Immediate
         * flush is slower but ensures that each append request is actually
         * written. If <code>immediateFlush</code> is set to
         * <code>false</code>, then there is a good chance that the last few
         * logs events are not actually written to persistent media if and
         * when the application crashes.
         *  
         * The <code>immediateFlush</code> variable is set to
         * <code>true</code> by default.
         */
        bool immediateFlush;

        log4cplus::tofstream out;
        log4cplus::tstring filename;

    private:
        void init(const log4cplus::tstring& filename,
                  LOG4CPLUS_OPEN_MODE_TYPE mode);

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
                            int maxBackupIndex = 1,
                            bool immediateFlush = true);
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
     */
    class LOG4CPLUS_EXPORT DailyRollingFileAppender : public FileAppender {
    public:
      // Ctors
        DailyRollingFileAppender(const log4cplus::tstring& filename,
                                 DailyRollingFileSchedule schedule = DAILY,
                                 bool immediateFlush = true,
                                 int maxBackupIndex = 10);
        DailyRollingFileAppender(const log4cplus::helpers::Properties& properties);

      // Dtor
        virtual ~DailyRollingFileAppender();
        
      // Methods
        virtual void close();

    protected:
        virtual void append(const spi::InternalLoggingEvent& event);
        void rollover();
        log4cplus::helpers::Time calculateNextRolloverTime(const log4cplus::helpers::Time& t) const;
        log4cplus::tstring getFilename(const log4cplus::helpers::Time& t) const;

      // Data
        DailyRollingFileSchedule schedule;
        log4cplus::tstring scheduledFilename;
        log4cplus::helpers::Time nextRolloverTime;
        int maxBackupIndex;

    private:
        void init(DailyRollingFileSchedule schedule);
    };

} // end namespace log4cplus

#endif // _LOG4CPLUS_FILE_APPENDER_HEADER_

