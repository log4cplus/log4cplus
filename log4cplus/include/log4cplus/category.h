// Module:  Log4CPLUS
// File:    category.h
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

#ifndef _LOG4CPLUS_CATEGORY_HEADER_
#define _LOG4CPLUS_CATEGORY_HEADER_

#include <log4cplus/config.h>
#include <log4cplus/priority.h>
#include <log4cplus/helpers/pointer.h>
#include <log4cplus/spi/appenderattachable.h>
#include <log4cplus/spi/categoryfactory.h>
#include <memory>
#include <string>
#include <sstream>
#include <vector>

namespace log4cplus {
    // Forward declarations
    namespace spi {
        struct InternalLoggingEvent;
        class CategoryImpl;
        typedef helpers::SharedObjectPtr<CategoryImpl> SharedCategoryImplPtr;
    }
    class Appender;
    class Hierarchy;
    class DefaultCategoryFactory;


    /** @var This is a list of {@link Category Categories}. */
    typedef std::vector<Category> CategoryList;


    /**
     * This is the central class in the log4cplus package. One of the
     * distintive features of log4cplus are hierarchical categories and their
     * evaluation.
     * <p>
     * See the <a href="../../../../manual.html">user manual</a> for an
     * introduction on this class.
     */
    class Category : public log4cplus::spi::AppenderAttachable {
    public:
      // Static Methods
        /**
         * Returns <code>true </code>if the named category exists 
         * (in the default hierarchy).
         *                
         * @param name The name of the category to search for.
         */
        static bool exists(const std::string& name);

        /*
         * Returns all the currently defined categories in the default
         * hierarchy.
         * <p>
         * The root category is <em>not</em> included in the returned
         * list.     
         */
        static CategoryList getCurrentCategories();
     
        /**
         * Return the default Hierarchy instance.
         */
        static Hierarchy& getDefaultHierarchy();

        /**
         * Retrieve a category with name <code>name</code>.  If the named 
         * category already exists, then the existing instance will be returned. 
         * Otherwise, a new instance is created. 
         * <p>
         * By default, categories do not have a set priority but inherit
         * it from the hierarchy. This is one of the central features of
         * log4cplus.
         * <p>
         * @param name The name of the category to retrieve.  
         */
        static Category getInstance(const std::string& name);

        /**
         * Like {@link #getInstance(std::string)} except that the type of category
         * instantiated depends on the type returned by the {@link
         * spi::CategoryFactory#makeNewCategoryInstance} method of the
         * <code>factory</code> parameter.
         * <p>                         
         * This method is intended to be used by sub-classes.
         * <p>                                  
         * @param name The name of the category to retrieve.
         * @param factory A {@link spi::CategoryFactory} implementation that will
         * actually create a new Instance.
         */
        static Category getInstance(const std::string& name, spi::CategoryFactory& factory);

        /**
         * Return the root of the default category hierrachy.
         * <p>
         * The root category is always instantiated and available. It's
         * name is "root".
         * <p>
         * Nevertheless, calling {@link #getInstance
         * Category.getInstance("root")} does not retrieve the root category 
         * but a category just under root named "root".
         */
        static Category getRoot();

        /**
         * Calling this method will <em>safely</em> close and remove all
         * appenders in all the categories including root contained in the
         * default hierachy.
         * <p>                    
         * Some appenders such as {@link org.apache.log4j.net.SocketAppender}
         * and {@link AsyncAppender} need to be closed before the
         * application exists. Otherwise, pending logging events might be
         * lost.
         * <p>
         * The <code>shutdown</code> method is careful to close nested
         * appenders before closing regular appenders. This is allows
         * configurations where a regular appender is attached to a category
         * and again to a nested appender.  
         */
        static void shutdown();

      // Non-Static Methods
        /**
         * If <code>assertion</code> parameter is <code>false</code>, then
         * logs <code>msg</code> as an {@link #error(const std::string&) error} 
         * statement.
         *
         * @param assertion 
         * @param msg The message to print if <code>assertion</code> is
         * false.
         */
        void assertion(bool assert, const std::string& msg);

        /**
         * Call the appenders in the hierrachy starting at
         * <code>this</code>.  If no appenders could be found, emit a
         * warning.
         * <p>
         * This method calls all the appenders inherited from the
         * hierarchy circumventing any evaluation of whether to log or not
         * to log the particular log request.
         *                                   
         * @param spi::InternalLoggingEvent the event to log. 
         */
        void callAppenders(const spi::InternalLoggingEvent& event);

        /**
         * Close all attached appenders implementing the AppenderAttachable
         * interface.  
         */
        void closeNestedAppenders();

        /**
         * Check whether this category is enabled for a given {@link
         * Priority} passed as parameter.
         *
         * See also {@link #isDebugEnabled}.
         *                      
         * @return boolean True if this category is enabled for <code>priority</code>.
         */
        bool isEnabledFor(const Priority& priority);

        /**
         *  Check whether this category is enabled for the {@link Priority#DEBUG_PRI}
         *  priority.
         *  <p>
         *  This function is intended to lessen the computational cost of
         *  disabled log debug statements.
         *  <p>
         *  For some <code>cat</code> Category object, when you write,
         *  <pre>
         *      cat.debug("This is entry name: " + n );
         *  </pre>
         *  
         *  <p>You incur the cost constructing the message, concatenatiion in
         *  this case, regardless of whether the message is logged or not.
         * 
         *  <p>If you are worried about speed, then you should write
         *  <pre>
         *    if(cat.isDebugEnabled()) { 
         *      cat.debug("This is entry name: " + n );
         *    }
         *  </pre>
         * 
         *  <p>This way you will not incur the cost of parameter
         *  construction if debugging is disabled for <code>cat</code>. On
         *  the other hand, if the <code>cat</code> is debug enabled, you
         *  will incur the cost of evaluating whether the category is debug
         *  enabled twice. Once in <code>isDebugEnabled</code> and once in
         *  the <code>debug</code>.  This is an insignificant overhead
         *  since evaluating a category takes about 1% of the time it
         *  takes to actually log.
         * 
         *  @return boolean - <code>true</code> if this category is debug
         *  enabled, <code>false</code> otherwise.
         */
        bool isDebugEnabled();

        /**
         * Check whether this category is enabled for the info priority.
         * See also {@link #isDebugEnabled}.
         *
         * @return boolean - <code>true</code> if this category is enabled
         * for priority info, <code>false</code> otherwise.
         */
        bool isInfoEnabled();

        /**
         * Check whether this category is enabled for the warn priority.
         * See also {@link #isDebugEnabled}.
         *
         * @return boolean - <code>true</code> if this category is enabled
         * for priority info, <code>false</code> otherwise.
         */
        bool isWarnEnabled();

        /**
         * This generic form is intended to be used by wrappers. 
         */
        void log(const Priority& priority, const std::string& message,
                 const char* file=NULL, int line=-1);
        void log(const Priority::PriorityLevel p, const std::string& message,
                 const char* file=NULL, int line=-1)
            { log(Priority(p), message, file, line); }

        /** 
         * Log a message with the {@link Priority#FATAL_PRI FATAL_PRI} priority.
         * <p>
         * This method first checks if this category is <code>FATAL</code>
         * enabled by comparing the priority of this category with the {@link
         * Priority#FATAL_PRI FATAL_PRI} priority. If this category is
         * <code>FATAL</code> enabled, then it proceeds to call all the
         * registered appenders in this category and also higher in the
         * hierarchy depending on the value of the additivity flag.
         *
         * @param message the message object to log. 
         */
        void fatal(const std::string& message, const char* file=NULL, int line=-1);

        /** 
         * Log a message with the {@link Priority#ERROR_PRI ERROR_PRI} priority.
         * <p>
         * This method first checks if this category is <code>ERROR</code>
         * enabled by comparing the priority of this category with the {@link
         * Priority#ERROR_PRI ERROR_PRI} priority. If this category is
         * <code>ERROR</code> enabled, then it proceeds to call all the
         * registered appenders in this category and also higher in the
         * hierarchy depending on the value of the additivity flag.
         *
         * @param message the message object to log. 
         */
        void error(const std::string& message, const char* file=NULL, int line=-1);

        /** 
         * Log a message with the {@link Priority#WARN_PRI WARN_PRI} priority.
         * <p>
         * This method first checks if this category is <code>WARN</code>
         * enabled by comparing the priority of this category with the {@link
         * Priority#WARN_PRI WARN_PRI} priority. If this category is
         * <code>WARN</code> enabled, then it proceeds to call all the
         * registered appenders in this category and also higher in the
         * hierarchy depending on the value of the additivity flag.
         * 
         * @param message the message object to log. 
         */
        void warn(const std::string& message, const char* file=NULL, int line=-1);

        /** 
         * Log a message with the {@link Priority#INFO_PRI INFO_PRI} priority.
         *
         * <p>This method first checks if this category is <code>INFO</code>
         * enabled by comparing the priority of this category with the {@link
         * Priority#INFO_PRI INFO_PRI} priority. If this category is
         * <code>INFO</code> enabled, then it proceeds to call all the
         * registered appenders in this category and also higher in the
         * hierarchy depending on the value of the additivity flag.
         *
         * @param message the message object to log. 
         */
        void info(const std::string& message, const char* file=NULL, int line=-1);

        /** 
         * Log a message with the {@link Priority#DEBUG_PRI DEBUG_PRI} priority.
         *
         * <p>This method first checks if this category is <code>DEBUG</code>
         * enabled by comparing the priority of this category with the {@link
         * Priority#DEBUG_PRI DEBUG_PRI} priority. If this category is
         * <code>DEBUG</code> enabled, then it proceeds to call all the
         * registered appenders in this category and also higher in the
         * hierarchy depending on the value of the additivity flag.
         *
         * @param message the message object to log. 
         */
        void debug(const std::string& message, const char* file=NULL, int line=-1);

        /**
         * Starting from this category, search the category hierarchy for a
         * non-NULL priority and return it. Otherwise, return the priority of the
         * root category.
         * <p>
         * The Category class is designed so that this method executes as
         * quickly as possible.
         */
        Priority getChainedPriority() const;

        /**
         * Returns the assigned {@link Priority}, if any, for this Category.  
         *           
         * @return Priority - the assigned Priority, can be <code>NULL</code>.
         */
        const Priority* getPriority() const;

        /**
         * Set the priority of this Category.
         * <p>
         * NULL values are admitted.
         */
        void setPriority(const Priority* priority);
        void setPriority(Priority::PriorityLevel p) 
            { Priority tmp(p); setPriority(&tmp); };

        /**
         * Return the the {@link Hierarchy} where this <code>Category</code> instance is
         * attached.
         */
        Hierarchy& getHierarchy() const;

        /**
         * Return the category name.  
         */
        std::string getName() const;

        /**
         * Get the additivity flag for this Category instance.  
         */
        bool getAdditivity() const;

        /**
         * Set the additivity flag for this Category instance.
         */
        void setAdditivity(bool additive);


      // AppenderAttachable Methods
        virtual void addAppender(SharedAppenderPtr newAppender);

        virtual SharedAppenderPtrList getAllAppenders();

        virtual SharedAppenderPtr getAppender(const std::string& name);

        virtual void removeAllAppenders();

        virtual void removeAppender(SharedAppenderPtr appender);

        virtual void removeAppender(const std::string& name);

      // Copy Ctor
        Category(const Category& rhs);
        Category& operator=(const Category& rhs);

      // Dtor
        ~Category();

        /**
         * Used to retrieve the parent of this Category in the
         * Category tree.
         */
        Category getParent();

    protected:
      // Ctors
        /**
         * This constructor created a new <code>Category</code> instance 
         * with a pointer to a Category implementation.
         * <p>
         * It is intended to be used by sub-classes only. You should not
         * create categories directly.
         *
         * @param ptr A pointer to the Category implementation.  This value
         *            cannot be NULL.  
         */
        Category(spi::CategoryImpl *ptr);
        Category(const spi::SharedCategoryImplPtr& val);

      // Methods
        /**
         * This method creates a new logging event and logs the event
         * without further checks.  
         */
        void forcedLog(const Priority& priority, const std::string& message,
                       const char* file=NULL, int line=-1);

      // Data
        /** This is a pointer to the implementation class. */
        spi::CategoryImpl *value;

    private:
      // Methods
        void init();
        inline void validate(const char *file, int line) const;

      // Friends
        friend class spi::CategoryImpl;
        friend class Hierarchy;
        friend class DefaultCategoryFactory;
    };


    /**
     * This class is used to create the default implementation of
     * the Category class
     */
    class DefaultCategoryFactory : public spi::CategoryFactory {
    public:
        Category makeNewCategoryInstance(const std::string& name, Hierarchy& h);
    };


}; // end namespace log4cplus


/**
 * @def LOG4CPLUS_DEBUG(cat, log)  This macro is used to log an 
 * {@link Priority#DEBUG_PRI DEBUG_PRI} message to <code>cat</code>.  
 * <code>log</code> will be streamed into an <code>ostream</code>.
 * 
 */
#define LOG4CPLUS_DEBUG(cat, log) \
    if(cat.isDebugEnabled()) { \
        std::ostringstream _log4cplus_buf; \
        _log4cplus_buf << log; \
        cat.debug(_log4cplus_buf.str(), __FILE__, __LINE__); \
    }

#define LOG4CPLUS_INFO(cat, log) \
    if(cat.isInfoEnabled()) { \
        std::ostringstream _log4cplus_buf; \
        _log4cplus_buf << log; \
        cat.info(_log4cplus_buf.str(), __FILE__, __LINE__); \
    }

#define LOG4CPLUS_WARN(cat, log) \
    if(cat.isWarnEnabled()) { \
        std::ostringstream _log4cplus_buf; \
        _log4cplus_buf << log; \
        cat.info(_log4cplus_buf.str(), __FILE__, __LINE__); \
    }

#define LOG4CPLUS_ERROR(cat, log) \
    { \
        std::ostringstream _log4cplus_buf; \
        _log4cplus_buf << log; \
        cat.error(_log4cplus_buf.str(), __FILE__, __LINE__); \
    }

#define LOG4CPLUS_FATAL(cat, log) \
    { \
        std::ostringstream _log4cplus_buf; \
        _log4cplus_buf << log; \
        cat.fatal(_log4cplus_buf.str(), __FILE__, __LINE__); \
    }


#endif // _LOG4CPLUS_CATEGORY_HEADER_


