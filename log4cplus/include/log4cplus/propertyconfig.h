// Module:  Log4CPLUS
// File:    propertyconfig.h
// Created: 3/2003
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

#ifndef _PROPERTY_CONFIG_HEADER_
#define _PROPERTY_CONFIG_HEADER_

#include <log4cplus/config.h>
#include <log4cplus/appender.h>
#include <log4cplus/category.h>
#include <log4cplus/helpers/property.h>

#include <map>


namespace log4cplus {

    /**
     * Extends {@link BasicConfigurator} to provide configuration from an
     * external file.  See <b>{@link #doConfigure(String, Hierarchy)}</b> for the
     * expected format.
     * 
     * <p>It is sometimes useful to see how log4j is reading configuration
     * files. You can enable log4j internal logging by defining the
     * <b>log4j.debug</b> variable.
     * 
     * <p>As of log4j version 0.8.5, at the initialization of the Category
     * class, the file <b>log4j.properties</b> will be searched from the
     * search path used to load classes. If the file can be found, then it
     * will be fed to the {@link
     * PropertyConfigurator#configure(java.net.URL)} method.
     * 
     * <p>The <code>PropertyConfigurator</code> does not handle the
     * advanced configuration features supported by the {@link
     * org.apache.log4j.xml.DOMConfigurator DOMConfigurator} such as
     * support for {@link org.apache.log4j.spi.Filter Filters}, custom
     * {@link org.apache.log4j.spi.ErrorHandler ErrorHandlers}, nested
     * appenders such as the {@link org.apache.log4j.AsyncAppender
     * AsyncAppender}, etc.
     * 
     * <p><em>All option values admit variable substitution.</em> For
     * example, if <code>java.home</code> system property is set to
     * <code>/home/xyz</code> and the File option is set to the string
     * <code>${java.home}/test.log</code>, then File option will be
     * interpreted as the string <code>/home/xyz/test.log</code>.
     * 
     * <p>The value of the substituted variable can be defined as a system
     * property or in the configuration file file itself.
     * 
     * <p>The syntax of variable substitution is similar to that of UNIX
     * shells. The string between an opening <b>&quot;${&quot;</b> and
     * closing <b>&quot;}&quot;</b> is interpreted as a key. Its value is
     * searched in the system properties, and if not founf then in the
     * configuration file being parsed.  The corresponding value replaces
     * the ${variableName} sequence.
     */
    class PropertyConfigurator {
    public:
        // ctor and dtor
        PropertyConfigurator(const std::string& propertyFile);
        virtual ~PropertyConfigurator();

        /**
         * Read configuration from a file. <b>The existing configuration is
         * not cleared nor reset.</b> If you require a different behavior,
         * then call {@link BasicConfigurator#resetConfiguration
         * resetConfiguration} method before calling
         * <code>doConfigure</code>.
         *
         * <p>The configuration file consists of statements in the format
         * <code>key=value</code>. The syntax of different configuration
         * elements are discussed below.
         *
         * <h3>Appender configuration</h3>
         *
         * <p>Appender configuration syntax is:
         * <pre>
         * # For appender named <i>appenderName</i>, set its class.
         * # Note: The appender name can contain dots.
         * log4j.appender.appenderName=fully.qualified.name.of.appender.class
         *
         * # Set appender specific options.
         * log4j.appender.appenderName.option1=value1
         * ...
         * log4j.appender.appenderName.optionN=valueN
         * </pre>
         *
         * For each named appender you can configure its {@link Layout}. The
         * syntax for configuring an appender's layout is:
         * <pre>
         * log4j.appender.appenderName.layout=fully.qualified.name.of.layout.class
         * log4j.appender.appenderName.layout.option1=value1
         * ....
         * log4j.appender.appenderName.layout.optionN=valueN
         * </pre>
         *
         * <h3>Configuring categories</h3>
         *
         * <p>The syntax for configuring the root category is:
         * <pre>
         * log4j.rootCategory=[priority], appenderName, appenderName, ...
         * </pre>
         *
         * <p>This syntax means that an optional <em>priority value</em> can
         * be supplied followed by appender names separated by commas.
         *
         * <p>The priority value can consist of the string values FATAL,
         * ERROR, WARN, INFO, DEBUG or a <em>custom priority</em> value. A
         * custom priority value can be specified in the form
         * <code>priority#classname</code>.
         *
         * <p>If a priority value is specified, then the root priority is set
         * to the corresponding priority.  If no priority value is specified,
         * then the root priority remains untouched.
         *
         * <p>The root category can be assigned multiple appenders.
         *
         * <p>Each <i>appenderName</i> (separated by commas) will be added to
         * the root category. The named appender is defined using the
         * appender syntax defined above.
         *
         * <p>For non-root categories the syntax is almost the same:
         * <pre>
         * log4j.category.category_name=[priority|INHERITED], appenderName, appenderName, ...
         * </pre>
         *
         * <p>The meaning of the optional priority value is discussed above
         * in relation to the root category. In addition however, the value
         * INHERITED can be specified meaning that the named category should
         * inherit its priority from the category hierarchy.
         *
         * <p>If no priority value is supplied, then the priority of the
         * named category remains untouched.
         *
         * <p>By default categories inherit their priority from the
         * hierarchy. However, if you set the priority of a category and
         * later decide that that category should inherit its priority, then
         * you should specify INHERITED as the value for the priority value.
         *
         * <p>Similar to the root category syntax, each <i>appenderName</i>
         * (separated by commas) will be attached to the named category.
         *
         * <p>See the <a href="../../../../manual.html#additivity">appender
         * additivity rule</a> in the user manual for the meaning of the
         * <code>additivity</code> flag.
         *
         * <p>The user can override any of the {@link
         * Hierarchy#disable} family of methods by setting the a key
         * "log4j.disableOverride" to <code>true</code> or any value other
         * than false. As in <pre>log4j.disableOverride=true </pre>
         *
         * <h3>Class Factories</h3>
         *
         * In case you are using your own subtypes of the
         * <code>Category</code> class and wish to use configuration files,
         * then you <em>must</em> set the <code>categoryFactory</code> for
         * the subtype that you are using.
         *
         * <p>The syntax is:
         *
         * <pre>
         * log4j.categoryFactory=fully.qualified.name.of.categoryFactory.class
         * </pre>
         *
         * See {@link org.apache.log4j.examples.MyCategory} for an example.
         *
         * <h3>Example</h3>
         *
         * <p>An example configuration is given below. Other configuration
         * file examples are given in {@link org.apache.log4j.examples.Sort}
         * class documentation.
         *
         * <pre>
         *
         * # Set options for appender named "A1".
         * # Appender "A1" will be a SyslogAppender
         * log4j.appender.A1=org.apache.log4j.net.SyslogAppender
         *
         * # The syslog daemon resides on www.abc.net
         * log4j.appender.A1.SyslogHost=www.abc.net
         *
         * # A1's layout is a PatternLayout, using the conversion pattern
         * # <b>%r %-5p %c{2} %M.%L %x - %m\n</b>. Thus, the log output will
         * # include # the relative time since the start of the application in
         * # milliseconds, followed by the priority of the log request,
         * # followed by the two rightmost components of the category name,
         * # followed by the callers method name, followed by the line number,
         * # the nested disgnostic context and finally the message itself.
         * # Refer to the documentation of {@link PatternLayout} for further information
         * # on the syntax of the ConversionPattern key.
         * log4j.appender.A1.layout=org.apache.log4j.PatternLayout
         * log4j.appender.A1.layout.ConversionPattern=%-4r %-5p %c{2} %M.%L %x - %m\n
         *
         * # Set options for appender named "A2"
         * # A2 should be a RollingFileAppender, with maximum file size of 10 MB
         * # using at most one backup file. A2's layout is TTCC, using the
         * # ISO8061 date format with context printing enabled.
         * log4j.appender.A2=org.apache.log4j.RollingFileAppender
         * log4j.appender.A2.MaxFileSize=10MB
         * log4j.appender.A2.MaxBackupIndex=1
         * log4j.appender.A2.layout=org.apache.log4j.TTCCLayout
         * log4j.appender.A2.layout.ContextPrinting=enabled
         * log4j.appender.A2.layout.DateFormat=ISO8601
         *
         * # Root category set to DEBUG using the A2 appender defined above.
         * log4j.rootCategory=DEBUG, A2
         *
         * # Category definitions:
         * # The SECURITY category inherits is priority from root. However, it's output
         * # will go to A1 appender defined above. It's additivity is non-cumulative.
         * log4j.category.SECURITY=INHERIT, A1
         * log4j.additivity.SECURITY=false
         *
         * # Only warnings or above will be logged for the category "SECURITY.access".
         * # Output will go to A1.
         * log4j.category.SECURITY.access=WARN
         *
         *
         * # The category "class.of.the.day" inherits its priority from the
         * # category hierarchy.  Output will go to the appender's of the root
         * # category, A2 in this case.
         * log4j.category.class.of.the.day=INHERIT
         * </pre>
         *
         * <p>Refer to the <b>setOption</b> method in each Appender and
         * Layout for class specific options.
         *
         * <p>Use the <code>#</code> or <code>!</code> characters at the
         * beginning of a line for comments.
         *
         * <p>
         * @param configFileName The name of the configuration file where the
         *                       configuration information is stored.
         */
        virtual void configure();

    protected:
	// Methods
	void configureCategories();
	void configureCategory(log4cplus::Category cat, const std::string& config);
	void configureAppenders();
	void configureAdditivity();

	// Types
	typedef std::map<std::string, log4cplus::SharedAppenderPtr> AppenderMap;

	// Data
	std::string propertyFilename;
	log4cplus::helpers::Properties properties; 
	AppenderMap appenders;
    };

    
}; // end namespace log4cplus

#endif // _PROPERTY_CONFIG_HEADER_

