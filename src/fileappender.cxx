// Module:  Log4CPLUS
// File:    fileappender.cxx
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

#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggingevent.h>
#include <algorithm>
#include <sstream>
#include <stdio.h>

using namespace std;
using namespace log4cplus::helpers;

#define MINIMUM_ROLLING_LOG_SIZE 200*1024L


log4cplus::FileAppender::FileAppender(const std::string& filename, 
                                      std::ios::open_mode mode)
: out(filename.c_str(), mode),
  filename(filename)
{
    if(!out.is_open()) {
        getErrorHandler()->error("Unable to open file: " + filename);
        return;
    }

    int columns = 80;
    std::string tmp;
    tmp.resize(columns);
    std::fill_n(tmp.begin(), columns, '#');
    
    out << tmp << std::endl
        << "#" << std::endl
        << "# Created Appender on: " 
        << getFormattedTime(time(NULL), "%m-%d-%y %H:%M:%S") << std::endl
        << "#" << std::endl
        << tmp << std::endl;

}


log4cplus::FileAppender::~FileAppender()
{
    destructorImpl();
}


void 
log4cplus::FileAppender::close()
{
    LOG4CPLUS_BEGIN_SYNCHRONIZE_ON_MUTEX( access_mutex )
        out.close();
    LOG4CPLUS_END_SYNCHRONIZE_ON_MUTEX
}



// This method does not need to be locked since it is called by
// doAppend() which performs the locking
void
log4cplus::FileAppender::append(const spi::InternalLoggingEvent& event)
{
    if(!out.is_open()) {
        getErrorHandler()->error("file is not open: " + filename);
        return;
    }

    layout->formatAndAppend(out, event);
}




log4cplus::RollingFileAppender::RollingFileAppender(const std::string& filename,
                                                    long maxFileSize,
                                                    int maxBackupIndex)
 : FileAppender(filename, std::ios::app),
   maxFileSize(max(maxFileSize, MINIMUM_ROLLING_LOG_SIZE)),
   maxBackupIndex(maxBackupIndex)
{
}



log4cplus::RollingFileAppender::~RollingFileAppender()
{
    destructorImpl();
}



// This method does not need to be locked since it is called by
// doAppend() which performs the locking
void
log4cplus::RollingFileAppender::append(const spi::InternalLoggingEvent& event)
{
    if(!out.is_open()) {
        getErrorHandler()->error("file is not open: " + filename);
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
        std::ostringstream buffer;
        buffer << filename << '.' << maxBackupIndex;
        remove(buffer.str().c_str());

        // Map {(maxBackupIndex - 1), ..., 2, 1} to {maxBackupIndex, ..., 3, 2}
        for(int i=maxBackupIndex - 1; i >= 1; i--) {
            std::ostringstream source;
            std::ostringstream target;

            source << filename << '.' << i;
            target << filename << '.' << (i+1);
            getLogLog().debug("Renaming file " + source.str() + " to " + target.str());
            rename(source.str().c_str(), target.str().c_str());
        }

        // Close the current file
        out.close();
        out.clear(); // reset flags since the C++ standard specified that all the
                     // flags should remain unchanged on a close

        // Rename fileName to fileName.1
        std::string target = filename + ".1";
        getLogLog().debug("Renaming file " + filename + " to " + target);
        rename(filename.c_str(), target.c_str());

        // Open a new file
        out.open(filename.c_str(), std::ios::out | std::ios::trunc);
    }
    else {
        getLogLog().debug(filename + " has no backups specified");
        // Close the current file
        out.close();
        out.clear(); // reset flags since the C++ standard specified that all the
                     // flags should remain unchanged on a close

        // Open it up again in truncation mode
        out.open(filename.c_str(), std::ios::out | std::ios::trunc);
    }
}

