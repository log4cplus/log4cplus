// Module:  Log4CPLUS
// File:    Layout.h
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

#ifndef _LOG4CPLUS_LAYOUT_HEADER_
#define _LOG4CPLUS_LAYOUT_HEADER_

#include <log4cplus/config.h>
#include <iostream>
#include <string>
#include <time.h>


namespace log4cplus {

    // Forward Declarations
    namespace spi {
        struct InternalLoggingEvent;
    }


    std::string getFormattedTime(time_t time, const std::string& fmt);


    /**
     * This class is used to layout strings sent to an {@link
     * #Appender}.
     */
    class Layout{
    public:
        virtual void formatAndAppend(std::ostream& output, 
                                     const log4cplus::spi::InternalLoggingEvent& event) = 0;
    };



    /**
     * SimpleLayout consists of the priority of the log statement,
     * followed by " - " and then the log message itself. For example,
     *
     * <pre>
     *         DEBUG - Hello world
     * </pre>
     *
     * <p>
     *
     * <p>{@link PatternLayout} offers a much more powerful alternative.
     */
    class SimpleLayout : public Layout {
    public:
        virtual void formatAndAppend(std::ostream& output, 
                                     const log4cplus::spi::InternalLoggingEvent& event);
    };



    /**
     * TTCC layout format consists of time, thread, category and nested
     * diagnostic context information, hence the name.
     * 
     * <p>Each of the four fields can be individually enabled or
     * disabled. The time format depends on the <code>DateFormat</code>
     * used.
     * 
     * <p>Here is an example TTCCLayout output with the {@link RelativeTimeDateFormat}.
     * 
     * <pre>
     * 176 [main] INFO  org.apache.log4j.examples.Sort - Populating an array of 2 elements in reverse order.
     * 225 [main] INFO  org.apache.log4j.examples.SortAlgo - Entered the sort method.
     * 262 [main] DEBUG org.apache.log4j.examples.SortAlgo.OUTER i=1 - Outer loop.
     * 276 [main] DEBUG org.apache.log4j.examples.SortAlgo.SWAP i=1 j=0 - Swapping intArray[0] = 1 and intArray[1] = 0
     * 290 [main] DEBUG org.apache.log4j.examples.SortAlgo.OUTER i=0 - Outer loop.
     * 304 [main] INFO  org.apache.log4j.examples.SortAlgo.DUMP - Dump of interger array:
     * 317 [main] INFO  org.apache.log4j.examples.SortAlgo.DUMP - Element [0] = 0
     * 331 [main] INFO  org.apache.log4j.examples.SortAlgo.DUMP - Element [1] = 1
     * 343 [main] INFO  org.apache.log4j.examples.Sort - The next log statement should be an error message.
     * 346 [main] ERROR org.apache.log4j.examples.SortAlgo.DUMP - Tried to dump an uninitialized array.
     *        at org.apache.log4j.examples.SortAlgo.dump(SortAlgo.java:58)
     *        at org.apache.log4j.examples.Sort.main(Sort.java:64)
     * 467 [main] INFO  org.apache.log4j.examples.Sort - Exiting main method.
     * </pre>
     * 
     *  <p>The first field is the number of milliseconds elapsed since the
     *  start of the program. The second field is the thread outputting the
     *  log statement. The third field is the priority, the fourth field is
     *  the category to which the statement belongs.
     * 
     *  <p>The fifth field (just before the '-') is the nested diagnostic
     *  context.  Note the nested diagnostic context may be empty as in the
     *  first two statements. The text after the '-' is the message of the
     *  statement.
     * 
     *  <p><b>WARNING</b> Do not use the same TTCCLayout instance from
     *  within different appenders. The TTCCLayout is not thread safe when
     *  used in his way. However, it is perfectly safe to use a TTCCLayout
     *  instance from just one appender.
     * 
     *  <p>{@link PatternLayout} offers a much more flexible alternative.
     */
    class TTCCLayout : public Layout {
    public:
      // Ctor
        TTCCLayout();

        virtual void formatAndAppend(std::ostream& output, 
                                     const log4cplus::spi::InternalLoggingEvent& event);

    protected:
        std::string dateFormat;
    };

}; // end namespace log4cplus

#endif // _LOG4CPLUS_LAYOUT_HEADER_

