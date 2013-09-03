lib_LTLIBRARIES = liblog4cplus.la

AM_CPPFLAGS = -DINSIDE_LOG4CPLUS \
		-I$(top_srcdir)/include -I$(top_builddir)/include \
		@LOG4CPLUS_NDEBUG@

AM_CXXFLAGS=-no-suppress @LOG4CPLUS_PROFILING_CXXFLAGS@

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
	appenderattachableimpl.cxx \
	appender.cxx \
	asyncappender.cxx \
	clogger.cxx \
	configurator.cxx \
	consoleappender.cxx \
	cygwin-win32.cxx \
	env.cxx \
	factory.cxx \
	fileappender.cxx \
	fileinfo.cxx \
	filter.cxx \
	global-init.cxx \
	hierarchy.cxx \
	hierarchylocker.cxx \
	layout.cxx \
	log4judpappender.cxx \
	lockfile.cxx \
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
	stringhelper-iconv.cxx \
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

if ENABLE_VERSION_INFO_OPTION
VERSION_INFO_OPTION=-version-info @LT_VERSION@
else
VERSION_INFO_OPTION=-avoid-version
endif

if ENABLE_RELEASE_OPTION
RELEASE_OPTION=-release @LT_RELEASE@
else
RELEASE_OPTION=
endif

LOG4CPLUS_VERSION_LT_FLAGS=$(VERSION_INFO_OPTION) $(RELEASE_OPTION)

#liblog4cplus_a_LIBADD = @LIBOBJS@ @ALLOCA@
liblog4cplus_la_LDFLAGS = -no-undefined \
	$(LOG4CPLUS_VERSION_LT_FLAGS) \
	$(LOG4CPLUS_AIX_XLC_LDFLAGS)
