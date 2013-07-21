lib_LTLIBRARIES = liblog4cplus.la

SINGLE_THREADED_SRC = \
	src/appenderattachableimpl.cxx \
	src/appender.cxx \
	src/asyncappender.cxx \
	src/clogger.cxx \
	src/configurator.cxx \
	src/connectorthread.cxx \
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

liblog4cplus_la_CPPFLAGS = $(AM_CPPFLAGS) -DINSIDE_LOG4CPLUS

#liblog4cplus_la_CXXFLAGS=@LOG4CPLUS_PROFILING_CXXFLAGS@

#liblog4cplus_la_LDFLAGS=@LOG4CPLUS_PROFILING_LDFLAGS@

liblog4cplus_la_SOURCES = $(LIB_SRC)

if USE_LOG4CPLUS_EXPORT_SYMBOLS_REGEX
LOG4CPLUS_EXPORT_SYMBOLS_REGEX = -export-symbols-regex 'log4cplus'
else
LOG4CPLUS_EXPORT_SYMBOLS_REGEX =
endif

#liblog4cplus_a_LIBADD = @LIBOBJS@ @ALLOCA@
liblog4cplus_la_LDFLAGS = -no-undefined \
	$(LOG4CPLUS_VERSION_LT_FLAGS) \
	@LOG4CPLUS_PROFILING_LDFLAGS@ \
	$(LOG4CPLUS_EXPORT_SYMBOLS_REGEX)
