// Module:  Log4CPLUS
// File:    hierarchy.h
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

#ifndef _LOG4CPLUS_HIERARCHY_HEADER_
#define _LOG4CPLUS_HIERARCHY_HEADER_

#include <log4cplus/config.h>
#include <log4cplus/category.h>
#include <log4cplus/helpers/pointer.h>
#include <map>
#include <memory>
#include <vector>


namespace log4cplus {

    /**
     * This class is specialized in retrieving categories by name and
     * also maintaining the category hierarchy.
     *
     * <p><em>The casual user should not have to deal with this class
     * directly.</em>  However, if you are in an environment where
     * multiple applications run in the same process, then read on.
     *
     * <p>The structure of the category hierarchy is maintained by the
     * {@link #getInstance} method. The hierarchy is such that children
     * link to their parent but parents do not have any pointers to their
     * children. Moreover, categories can be instantiated in any order, in
     * particular descendant before ancestor.
     *
     * <p>In case a descendant is created before a particular ancestor,
     * then it creates a provision node for the ancestor and adds itself
     * to the provision node. Other descendants of the same ancestor add
     * themselves to the previously created provision node.
     */
    class Hierarchy {
    public:
        // DISABLE_OFF should be set to a value lower than all possible
        // priorities.
        static const int DISABLE_OFF;
        static const int DISABLE_OVERRIDE;

      // Ctors
        /**
         * Create a new Category hierarchy.
         *
         * @param root The root of the new hierarchy.
         */
        Hierarchy();

      // Dtor
        virtual ~Hierarchy();

      // Methods
        /**
         * This call will clear all category definitions from the internal
         * hashtable. Invoking this method will irrevocably mess up the
         * category hierarchy.
         *                     
         * <p>You should <em>really</em> know what you are doing before
         * invoking this method.
         */
        virtual void clear();

        /**
         * Returns <code>true </code>if the named category exists 
         * (in the default hierarchy).
         *                
         * @param name The name of the category to search for.
         */
        virtual bool exists(const std::string& name);

        /**
         * Similar to {@link #disable(Priority)} except that the priority
         * argument is given as a std::string.  
         */
        virtual void disable(const std::string& priorityStr);

        /**
         * Disable all logging requests of priority <em>equal to or
         * below</em> the priority parameter <code>p</code>, for
         * <em>all</em> categories in this hierarchy. Logging requests of
         * higher priority then <code>p</code> remain unaffected.
         *
         * <p>Nevertheless, if the {@link
         * BasicConfigurator#DISABLE_OVERRIDE_KEY} system property is set to
         * "true" or any value other than "false", then logging requests are
         * evaluated as usual, i.e. according to the <a
         * href="../../../../manual.html#selectionRule">Basic Selection Rule</a>.
         *
         * <p>The "disable" family of methods are there for speed. They
         * allow printing methods such as debug, info, etc. to return
         * immediately after an integer comparison without walking the
         * category hierarchy. In most modern computers an integer
         * comparison is measured in nanoseconds where as a category walk is
         * measured in units of microseconds.
         *
         * <p>Other configurators define alternate ways of overriding the
         * disable override flag. See {@link PropertyConfigurator} and
         * {@link org.apache.log4j.xml.DOMConfigurator}.
         */
        virtual void disable(const Priority& p);
        void disable(Priority::PriorityLevel p) { disable(Priority(p)); };

        /**
         * Disable all logging requests regardless of category and priority.
         * This method is equivalent to calling {@link #disable} with the
         * argument {@link Priority#FATAL_PRI}, the highest possible priority.
         */
        virtual void disableAll();

        /**
         * Disable all Debug logging requests regardless of category.
         * This method is equivalent to calling {@link #disable} with the
         * argument {@link Priority#DEBUG_PRI}.
         */
        virtual void disableDebug();

        /**
         * Disable all Info logging requests regardless of category.
         * This method is equivalent to calling {@link #disable} with the
         * argument {@link Priority#INFO_PRI}.
         */
        virtual void disableInfo();

        /**
         * Undoes the effect of calling any of {@link #disable}, {@link
         * #disableAll}, {@link #disableDebug} and {@link #disableInfo}
         * methods. More precisely, invoking this method sets the Category
         * class internal variable called <code>disable</code> to its
         * default "off" value.
         */
        virtual void enableAll();

        /**
         * Return a new category instance named as the first parameter using
         * the default factory. 
         *                
         * <p>If a category of that name already exists, then it will be
         * returned.  Otherwise, a new category will be instantiated and
         * then linked with its existing ancestors as well as children.
         *                                    
         * @param name The name of the category to retrieve.
         */
        virtual Category getInstance(const std::string& name);

        /**
         * Return a new category instance named as the first parameter using
         * <code>factory</code>.
         *                
         * <p>If a category of that name already exists, then it will be
         * returned.  Otherwise, a new category will be instantiated by the
         * <code>factory</code> parameter and linked with its existing
         * ancestors as well as children.
         *                                         
         * @param name The name of the category to retrieve.
         * @param factory The factory that will make the new category instance.
         */
        virtual Category getInstance(const std::string& name, spi::CategoryFactory& factory);

        /**
         * Returns all the currently defined categories in this hierarchy.
         *
         * <p>The root category is <em>not</em> included in the returned list. 
         */
        virtual CategoryList getCurrentCategories();

        /** 
         * Is the {@link Priority} specified by <code>level</code> enabled? 
         */
        virtual bool isDisabled(int level);

        /**
         * Get the root of this hierarchy.
         */
        virtual Category getRoot();

        /**
         * Reset all values contained in this hierarchy instance to their
         * default.  This removes all appenders from all categories, sets
         * the priority of all non-root categories to <code>null</code>,
         * sets their additivity flag to <code>true</code> and sets the priority
         * of the root category to {@link Priority#DEBUG_PRI DEBUG_PRI}.  Moreover,
         * message disabling is set its default "off" value.
         *
         * <p>Existing categories are not removed. They are just reset.
         *
         * <p>This method should be used sparingly and with care as it will
         * block all logging until it is completed.</p>
         */
        virtual void resetConfiguration(); 

        /**
         * Set the default CategoryFactory instance.
         */
        virtual void setCategoryFactory(std::auto_ptr<spi::CategoryFactory> factory);

        /**
         * Shutting down a hierarchy will <em>safely</em> close and remove
         * all appenders in all categories including the root category.
         *                
         * <p>Some appenders such as {@link org.apache.log4j.net.SocketAppender}
         * and {@link AsyncAppender} need to be closed before the
         * application exists. Otherwise, pending logging events might be
         * lost.
         *
         * <p>The <code>shutdown</code> method is careful to close nested
         * appenders before closing regular appenders. This is allows
         * configurations where a regular appender is attached to a category
         * and again to a nested appender.
         */
        virtual void shutdown();

    private:
      // Types
        typedef std::vector<Category> ProvisionNode;
        typedef std::map<std::string, ProvisionNode> ProvisionNodeMap;
        typedef std::map<std::string, Category> CategoryMap;

      // Methods
        /**
         * This method loops through all the *potential* parents of
         * cat'. There 3 possible cases:
         *
         * 1) No entry for the potential parent of 'cat' exists
         *
         *    We create a ProvisionNode for this potential parent and insert
         *    'cat' in that provision node.
         *
         * 2) There is an entry of type Category for the potential parent.
         *
         *    The entry is 'cat's nearest existing parent. We update cat's
         *    parent field with this entry. We also break from the loop
         *    because updating our parent's parent is our parent's
         *    responsibility.
         *
         * 3) There entry is of type ProvisionNode for this potential parent.
         *
         *    We add 'cat' to the list of children for this potential parent.
         */
        void updateParents(Category cat);

        /**
         * We update the links for all the children that placed themselves
         * in the provision node 'pn'. The second argument 'cat' is a
         * reference for the newly created Category, parent of all the
         * children in 'pn'
         *
         * We loop on all the children 'c' in 'pn':
         *
         *    If the child 'c' has been already linked to a child of
         *    'cat' then there is no need to update 'c'.
         *
         *   Otherwise, we set cat's parent field to c's parent and set
         *   c's parent field to cat.
         */
        void updateChildren(ProvisionNode& pn, Category cat);

    // Data
       LOG4CPLUS_MUTEX_PTR_DECLARE hashtable_mutex;
       std::auto_ptr<spi::CategoryFactory> defaultFactory;
       ProvisionNodeMap provisionNodes;
       CategoryMap categoryPtrs;
       Category *root;

       int disableValue;

       bool emittedNoAppenderWarning;
       bool emittedNoResourceBundleWarning;

    // Friends
        friend class spi::CategoryImpl;
    };


}; // end namespace log4cplus

#endif // _LOG4CPLUS_HIERARCHY_HEADER_

