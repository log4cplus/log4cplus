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
                     std::ios::openmode mode = std::ios::trunc);
        FileAppender(const log4cplus::helpers::Properties properties,
                     std::ios::openmode mode = std::ios::trunc);

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
        void init(const log4cplus::tstring& filename,
                  std::ios::openmode mode);
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
        RollingFileAppender(const log4cplus::helpers::Properties properties);

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

} // end namespace log4cplus

#endif // _LOG4CPLUS_FILE_APPENDER_HEADER_

