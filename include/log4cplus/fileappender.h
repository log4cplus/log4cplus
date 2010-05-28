// Module:  Log4CPLUS
// File:    fileappender.h
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

/** @file */

#ifndef _LOG4CPLUS_FILE_APPENDER_HEADER_
#define _LOG4CPLUS_FILE_APPENDER_HEADER_

#include <log4cplus/config.hxx>
#include <log4cplus/appender.h>
#include <log4cplus/fstreams.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/helpers/timehelper.h>

#if defined(__DECCXX)
#   define LOG4CPLUS_OPEN_MODE_TYPE LOG4CPLUS_FSTREAM_NAMESPACE::ios::open_mode
#else
#   define LOG4CPLUS_OPEN_MODE_TYPE LOG4CPLUS_FSTREAM_NAMESPACE::ios::openmode
#endif

namespace log4cplus {

    /**
     * Appends log events to a file.
     * 
     * <h3>Properties</h3>
     * <dl>
     * <dt><tt>File</tt></dt>
     * <dd>This property specifies output file name.</dd>
     * 
     * <dt><tt>ImmediateFlush</tt></dt>
     * <dd>When it is set true, output stream will be flushed after
     * each appended event.</dd>
     *
     * <dt><tt>Append</tt></dt>
     * <dd>When it is set true, output file will be appended to
     * instead of being truncated at opening.</dd>
     *
     * <dt><tt>ReopenDelay</tt></dt>
     * <dd>This property sets a delay after which the appender will
     * try to reopen log file again, after last logging failure. The
     * default value is 1 second. Setting the delay to 0 makes the
     * appender not to try reopening the stream.
     * </dd>
     *
     * <dt><tt>BufferSize</tt></dt>
     * <dd>Non-zero value of this property sets up buffering of output
     * stream using a buffer of given size.
     * </dd>
     * </dl>
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

        void open(LOG4CPLUS_OPEN_MODE_TYPE mode);
        bool reopen();

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

        /**
         * When any append operation fails, <code>reopenDelay</code> says 
         * for how many seconds the next attempt to re-open the log file and 
         * resume logging will be delayed. If <code>reopenDelay</code> is zero, 
         * each failed append operation will cause log file to be re-opened. 
         * By default, <code>reopenDelay</code> is 1 second.
         */
        int reopenDelay;

        unsigned long bufferSize;
        log4cplus::tchar * buffer;

        log4cplus::tofstream out;
        log4cplus::tstring filename;

        log4cplus::helpers::Time reopen_time;

    private:
        void init(const log4cplus::tstring& filename,
                  LOG4CPLUS_OPEN_MODE_TYPE mode);

      // Disallow copying of instances of this class
        FileAppender(const FileAppender&);
        FileAppender& operator=(const FileAppender&);
    };



    /**
     * RollingFileAppender extends FileAppender to backup the log
     * files when they reach a certain size.
     *
     * <h3>Properties</h3>
     * <p>Properties additional to {@link FileAppender}'s properties:
     *
     * <dl>
     * <dt><tt>MaxFileSize</tt></dt>
     * <dd>This property specifies maximal size of output file. The
     * value is in bytes. It is possible to use <tt>MB</tt> and
     * <tt>KB</tt> suffixes to specify the value in megabytes or
     * kilobytes instead.</dd>
     *
     * <dt><tt>MaxBackupIndex</tt></dt>
     * <dd>This property limits the number of backup output
     * files; e.g. how many <tt>log.1</tt>, <tt>log.2</tt> etc. files
     * will be kept.</dd>
     * </dl>
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
     * <h3>Properties</h3>
     * <p>Properties additional to {@link FileAppender}'s properties:
     *
     * <dl>
     * <dt><tt>Schedule</tt></dt>
     * <dd>This property specifies rollover schedule. The possible
     * values are <tt>MONTHLY</tt>, <tt>WEEKLY</tt>, <tt>DAILY</tt>,
     * <tt>TWICE_DAILY</tt>, <tt>HOURLY</tt> and
     * <tt>MINUTELY</tt>.</dd>
     *
     * <dt><tt>MaxBackupIndex</tt></dt>
     * <dd>This property limits how many backup files are kept per
     * single logging period; e.g. how many <tt>log.2009-11-07.1</tt>,
     * <tt>log.2009-11-07.2</tt> etc. files are kept.</dd>
     *
     * </dl>
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

