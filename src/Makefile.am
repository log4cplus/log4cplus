lib_LTLIBRARIES = liblog4cplus.la

AM_CPPFLAGS = -DINSIDE_LOG4CPLUS \
        -I$(top_srcdir)/include -I$(top_builddir)/include \
        @LOG4CPLUS_NDEBUG@

AM_LDFLAGS=@LOG4CPLUS_PROFILING_LDFLAGS@

AM_CXXFLAGS=-no-suppress @LOG4CPLUS_PROFILING_CXXFLAGS@

nobase_include_HEADERS = ../include/log4cplus/appender.h \
                         ../include/log4cplus/asyncappender.h \
                         ../include/log4cplus/boost/deviceappender.hxx \
                         ../include/log4cplus/config.hxx \
                         ../include/log4cplus/config/defines.hxx \
                         ../include/log4cplus/config/macosx.h \
                         ../include/log4cplus/config/win32.h \
                         ../include/log4cplus/config/windowsh-inc.h \
                         ../include/log4cplus/configurator.h \
                         ../include/log4cplus/consoleappender.h \
                         ../include/log4cplus/fileappender.h \
                         ../include/log4cplus/fstreams.h \
                         ../include/log4cplus/helpers/appenderattachableimpl.h \
                         ../include/log4cplus/helpers/loglog.h \
                         ../include/log4cplus/helpers/logloguser.h \
                         ../include/log4cplus/helpers/pointer.h \
                         ../include/log4cplus/helpers/property.h \
                         ../include/log4cplus/helpers/queue.h \
                         ../include/log4cplus/helpers/sleep.h \
                         ../include/log4cplus/helpers/snprintf.h \
                         ../include/log4cplus/helpers/socketbuffer.h \
                         ../include/log4cplus/helpers/socket.h \
                         ../include/log4cplus/helpers/stringhelper.h \
                         ../include/log4cplus/helpers/thread-config.h \
                         ../include/log4cplus/helpers/timehelper.h \
                         ../include/log4cplus/hierarchy.h \
                         ../include/log4cplus/hierarchylocker.h \
                         ../include/log4cplus/internal/env.h \
                         ../include/log4cplus/internal/internal.h \
                         ../include/log4cplus/internal/socket.h \
                         ../include/log4cplus/layout.h \
                         ../include/log4cplus/logger.h \
                         ../include/log4cplus/loggingmacros.h \
                         ../include/log4cplus/loglevel.h \
                         ../include/log4cplus/mdc.h \
                         ../include/log4cplus/ndc.h \
                         ../include/log4cplus/nteventlogappender.h \
                         ../include/log4cplus/nullappender.h \
                         ../include/log4cplus/spi/appenderattachable.h \
                         ../include/log4cplus/spi/factory.h \
                         ../include/log4cplus/spi/filter.h \
                         ../include/log4cplus/spi/loggerfactory.h \
                         ../include/log4cplus/spi/loggerimpl.h \
                         ../include/log4cplus/spi/loggingevent.h \
                         ../include/log4cplus/spi/objectregistry.h \
                         ../include/log4cplus/spi/rootlogger.h \
                         ../include/log4cplus/socketappender.h \
                         ../include/log4cplus/streams.h \
                         ../include/log4cplus/syslogappender.h \
                         ../include/log4cplus/tchar.h \
                         ../include/log4cplus/thread/impl/syncprims-impl.h \
                         ../include/log4cplus/thread/impl/syncprims-pthreads.h \
                         ../include/log4cplus/thread/impl/syncprims-win32.h \
                         ../include/log4cplus/thread/impl/threads-impl.h \
                         ../include/log4cplus/thread/impl/tls.h \
                         ../include/log4cplus/thread/syncprims.h \
                         ../include/log4cplus/thread/syncprims-pub-impl.h \
                         ../include/log4cplus/thread/threads.h \
                         ../include/log4cplus/tracelogger.h \
                         ../include/log4cplus/tstring.h \
                         ../include/log4cplus/version.h \
                         ../include/log4cplus/win32consoleappender.h \
                         ../include/log4cplus/win32debugappender.h

SINGLE_THREADED_SRC = appenderattachableimpl.cxx \
                      appender.cxx \
                      asyncappender.cxx \
                      configurator.cxx \
                      consoleappender.cxx \
                      env.cxx \
                      factory.cxx \
                      fileappender.cxx \
                      filter.cxx \
                      global-init.cxx \
                      hierarchy.cxx \
                      hierarchylocker.cxx \
                      layout.cxx \
                      logger.cxx \
                      loggerimpl.cxx \
                      loggingevent.cxx \
                      loggingmacros.cxx \
                      loglevel.cxx \
                      loglog.cxx \
                      logloguser.cxx \
                      mdc.cxx \
                      ndc.cxx \
                      nullappender.cxx \
                      nteventlogappender.cxx \
                      objectregistry.cxx \
                      patternlayout.cxx \
                      pointer.cxx \
                      property.cxx \
                      queue.cxx \
                      rootlogger.cxx \
                      sleep.cxx \
                      snprintf.cxx \
                      socketappender.cxx \
                      socketbuffer.cxx \
                      socket.cxx \
                      socket-unix.cxx \
                      socket-win32.cxx \
                      stringhelper.cxx \
                      stringhelper-clocale.cxx \
                      stringhelper-cxxlocale.cxx \
                      syncprims.cxx \
                      syslogappender.cxx \
                      threads.cxx \
                      timehelper.cxx \
                      tls.cxx \
                      version.cxx \
                      win32consoleappender.cxx \
                      win32debugappender.cxx

LIB_SRC = $(SINGLE_THREADED_SRC)

liblog4cplus_la_SOURCES = $(LIB_SRC)

#liblog4cplus_a_LIBADD = @LIBOBJS@ @ALLOCA@
liblog4cplus_la_LDFLAGS = -no-undefined -version-info @LT_VERSION@ -release 1.1
