// Module:  Log4CPLUS
// File:    fileappender.h
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright (C) The Apache Software Foundation. All rights reserved.
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
#include <fstream>
//#include "log4cplus/helpers/loglog.h"

namespace log4cplus {

    /**
     * Appends log events to a file. 
     */
    class FileAppender : public Appender {
    public:
      // Ctors
        FileAppender(const std::string& filename, 
                     std::ios::openmode mode = std::ios::trunc);

      // Dtor
        virtual ~FileAppender();

      // Methods
        virtual void close();

    protected:
        virtual void append(const spi::InternalLoggingEvent& event);

      // Data
        std::ofstream out;
        std::string filename;
    };



    /**
     * RollingFileAppender extends FileAppender to backup the log files when 
     * they reach a certain size. 
     */
    class RollingFileAppender : public FileAppender {
    public:
      // Ctors
        RollingFileAppender(const std::string& filename,
                            long maxFileSize = 10*1024*1024, // 10 MB
                            int maxBackupIndex = 1);

      // Dtor
        virtual ~RollingFileAppender();

    protected:
        virtual void append(const spi::InternalLoggingEvent& event);
        void rollover();

      // Data
        long maxFileSize;
        int maxBackupIndex;
    };

}; // end namespace log4cplus

#endif // _LOG4CPLUS_FILE_APPENDER_HEADER_

