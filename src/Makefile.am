lib_LTLIBRARIES = liblog4cplus.la

AM_CPPFLAGS = -DINSIDE_LOG4CPLUS \
		-I$(top_srcdir)/include -I$(top_builddir)/include \
		@LOG4CPLUS_NDEBUG@

AM_CXXFLAGS=@LOG4CPLUS_PROFILING_CXXFLAGS@

AM_LDFLAGS=@LOG4CPLUS_PROFILING_LDFLAGS@

INCLUDES_SRC_PATH = $(top_srcdir)/include/log4cplus

INCLUDES_SRC = \
	$(INCLUDES_SRC_PATH)/appender.h \
	$(INCLUDES_SRC_PATH)/asyncappender.h \
	$(INCLUDES_SRC_PATH)/boost/deviceappender.hxx \
	$(INCLUDES_SRC_PATH)/clogger.h \
	$(INCLUDES_SRC_PATH)/config.hxx \
	$(INCLUDES_SRC_PATH)/config/macosx.h \
	$(INCLUDES_SRC_PATH)/config/win32.h \
	$(INCLUDES_SRC_PATH)/config/windowsh-inc.h \
	$(INCLUDES_SRC_PATH)/configurator.h \
	$(INCLUDES_SRC_PATH)/consoleappender.h \
	$(INCLUDES_SRC_PATH)/fileappender.h \
	$(INCLUDES_SRC_PATH)/fstreams.h \
	$(INCLUDES_SRC_PATH)/helpers/appenderattachableimpl.h \
	$(INCLUDES_SRC_PATH)/helpers/fileinfo.h \
	$(INCLUDES_SRC_PATH)/helpers/lockfile.h \
	$(INCLUDES_SRC_PATH)/helpers/loglog.h \
	$(INCLUDES_SRC_PATH)/helpers/logloguser.h \
	$(INCLUDES_SRC_PATH)/helpers/pointer.h \
	$(INCLUDES_SRC_PATH)/helpers/property.h \
	$(INCLUDES_SRC_PATH)/helpers/queue.h \
	$(INCLUDES_SRC_PATH)/helpers/sleep.h \
	$(INCLUDES_SRC_PATH)/helpers/snprintf.h \
	$(INCLUDES_SRC_PATH)/helpers/socketbuffer.h \
	$(INCLUDES_SRC_PATH)/helpers/socket.h \
	$(INCLUDES_SRC_PATH)/helpers/stringhelper.h \
	$(INCLUDES_SRC_PATH)/helpers/thread-config.h \
	$(INCLUDES_SRC_PATH)/helpers/timehelper.h \
	$(INCLUDES_SRC_PATH)/hierarchy.h \
	$(INCLUDES_SRC_PATH)/hierarchylocker.h \
	$(INCLUDES_SRC_PATH)/internal/cygwin-win32.h \
	$(INCLUDES_SRC_PATH)/internal/env.h \
	$(INCLUDES_SRC_PATH)/internal/internal.h \
	$(INCLUDES_SRC_PATH)/internal/socket.h \
	$(INCLUDES_SRC_PATH)/layout.h \
	$(INCLUDES_SRC_PATH)/log4judpappender.h \
	$(INCLUDES_SRC_PATH)/logger.h \
	$(INCLUDES_SRC_PATH)/loggingmacros.h \
	$(INCLUDES_SRC_PATH)/loglevel.h \
	$(INCLUDES_SRC_PATH)/mdc.h \
	$(INCLUDES_SRC_PATH)/ndc.h \
	$(INCLUDES_SRC_PATH)/nteventlogappender.h \
	$(INCLUDES_SRC_PATH)/nullappender.h \
	$(INCLUDES_SRC_PATH)/spi/appenderattachable.h \
	$(INCLUDES_SRC_PATH)/spi/factory.h \
	$(INCLUDES_SRC_PATH)/spi/filter.h \
	$(INCLUDES_SRC_PATH)/spi/loggerfactory.h \
	$(INCLUDES_SRC_PATH)/spi/loggerimpl.h \
	$(INCLUDES_SRC_PATH)/spi/loggingevent.h \
	$(INCLUDES_SRC_PATH)/spi/objectregistry.h \
	$(INCLUDES_SRC_PATH)/spi/rootlogger.h \
	$(INCLUDES_SRC_PATH)/socketappender.h \
	$(INCLUDES_SRC_PATH)/streams.h \
	$(INCLUDES_SRC_PATH)/syslogappender.h \
	$(INCLUDES_SRC_PATH)/tchar.h \
	$(INCLUDES_SRC_PATH)/thread/impl/syncprims-impl.h \
	$(INCLUDES_SRC_PATH)/thread/impl/syncprims-pmsm.h \
	$(INCLUDES_SRC_PATH)/thread/impl/syncprims-pthreads.h \
	$(INCLUDES_SRC_PATH)/thread/impl/syncprims-win32.h \
	$(INCLUDES_SRC_PATH)/thread/impl/threads-impl.h \
	$(INCLUDES_SRC_PATH)/thread/impl/tls.h \
	$(INCLUDES_SRC_PATH)/thread/syncprims.h \
	$(INCLUDES_SRC_PATH)/thread/syncprims-pub-impl.h \
	$(INCLUDES_SRC_PATH)/thread/threads.h \
	$(INCLUDES_SRC_PATH)/tracelogger.h \
	$(INCLUDES_SRC_PATH)/tstring.h \
	$(INCLUDES_SRC_PATH)/version.h \
	$(INCLUDES_SRC_PATH)/win32consoleappender.h \
	$(INCLUDES_SRC_PATH)/win32debugappender.h \
	$(top_builddir)/include/log4cplus/config/defines.hxx

SINGLE_THREADED_SRC = \
	$(INCLUDES_SRC) \
	src/appenderattachableimpl.cxx \
	src/appender.cxx \
	src/asyncappender.cxx \
	src/clogger.cxx \
	src/configurator.cxx \
	src/consoleappender.cxx \
	src/cygwin-win32.cxx \
	src/env.cxx \
	src/factory.cxx \
	src/fileappender.cxx \
	src/fileinfo.cxx \
	src/filter.cxx \
	src/global-init.cxx \
	src/hierarchy.cxx \
	src/hierarchylocker.cxx \
	src/layout.cxx \
	src/log4judpappender.cxx \
	src/lockfile.cxx \
	src/logger.cxx \
	src/loggerimpl.cxx \
	src/loggingevent.cxx \
	src/loggingmacros.cxx \
	src/loglevel.cxx \
	src/loglog.cxx \
	src/logloguser.cxx \
	src/mdc.cxx \
	src/ndc.cxx \
	src/nullappender.cxx \
	src/nteventlogappender.cxx \
	src/objectregistry.cxx \
	src/patternlayout.cxx \
	src/pointer.cxx \
	src/property.cxx \
	src/queue.cxx \
	src/rootlogger.cxx \
	src/sleep.cxx \
	src/snprintf.cxx \
	src/socketappender.cxx \
	src/socketbuffer.cxx \
	src/socket.cxx \
	src/socket-unix.cxx \
	src/socket-win32.cxx \
	src/stringhelper.cxx \
	src/stringhelper-clocale.cxx \
	src/stringhelper-cxxlocale.cxx \
	src/stringhelper-iconv.cxx \
	src/syncprims.cxx \
	src/syslogappender.cxx \
	src/threads.cxx \
	src/timehelper.cxx \
	src/tls.cxx \
	src/version.cxx \
	src/win32consoleappender.cxx \
	src/win32debugappender.cxx

LIB_SRC = $(SINGLE_THREADED_SRC)

liblog4cplus_la_SOURCES = $(LIB_SRC)

#liblog4cplus_a_LIBADD = @LIBOBJS@ @ALLOCA@
liblog4cplus_la_LDFLAGS = -no-undefined -version-info @LT_VERSION@ -release @LT_RELEASE@
