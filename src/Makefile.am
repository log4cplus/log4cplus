lib_LTLIBRARIES = liblog4cplus.la

AM_CPPFLAGS = -DINSIDE_LOG4CPLUS \
	-I$(top_srcdir)/include -I$(top_builddir)/include \
	@LOG4CPLUS_NDEBUG@

AM_CXXFLAGS = -no-suppress

INCLUDES_SRC_PATH = $(top_srcdir)/include/log4cplus

INCLUDES_SRC = \
    $(INCLUDES_SRC_PATH)/appender.h \
	$(INCLUDES_SRC_PATH)/config.hxx \
	$(INCLUDES_SRC_PATH)/config/win32.h \
	$(INCLUDES_SRC_PATH)/config/macosx.h \
	$(INCLUDES_SRC_PATH)/configurator.h \
	$(INCLUDES_SRC_PATH)/consoleappender.h \
	$(INCLUDES_SRC_PATH)/fileappender.h \
	$(INCLUDES_SRC_PATH)/fstreams.h \
	$(INCLUDES_SRC_PATH)/hierarchy.h \
	$(INCLUDES_SRC_PATH)/hierarchylocker.h \
	$(INCLUDES_SRC_PATH)/layout.h \
	$(INCLUDES_SRC_PATH)/logger.h \
	$(INCLUDES_SRC_PATH)/loggingmacros.h \
	$(INCLUDES_SRC_PATH)/loglevel.h \
	$(INCLUDES_SRC_PATH)/ndc.h \
	$(INCLUDES_SRC_PATH)/nullappender.h \
	$(INCLUDES_SRC_PATH)/socketappender.h \
	$(INCLUDES_SRC_PATH)/streams.h \
	$(INCLUDES_SRC_PATH)/syslogappender.h \
	$(INCLUDES_SRC_PATH)/tstring.h \
	$(INCLUDES_SRC_PATH)/version.h \
	$(INCLUDES_SRC_PATH)/helpers/appenderattachableimpl.h \
	$(INCLUDES_SRC_PATH)/helpers/loglog.h \
	$(INCLUDES_SRC_PATH)/helpers/logloguser.h \
	$(INCLUDES_SRC_PATH)/helpers/pointer.h \
	$(INCLUDES_SRC_PATH)/helpers/property.h \
	$(INCLUDES_SRC_PATH)/helpers/sleep.h \
	$(INCLUDES_SRC_PATH)/helpers/socketbuffer.h \
	$(INCLUDES_SRC_PATH)/helpers/socket.h \
	$(INCLUDES_SRC_PATH)/helpers/stringhelper.h \
	$(INCLUDES_SRC_PATH)/helpers/syncprims.h \
	$(INCLUDES_SRC_PATH)/helpers/syncprims-pthreads.h \
	$(INCLUDES_SRC_PATH)/helpers/thread-config.h \
	$(INCLUDES_SRC_PATH)/helpers/threads.h \
	$(INCLUDES_SRC_PATH)/helpers/timehelper.h \
	$(INCLUDES_SRC_PATH)/spi/appenderattachable.h \
	$(INCLUDES_SRC_PATH)/spi/factory.h \
	$(INCLUDES_SRC_PATH)/spi/filter.h \
	$(INCLUDES_SRC_PATH)/spi/loggerfactory.h \
	$(INCLUDES_SRC_PATH)/spi/loggerimpl.h \
	$(INCLUDES_SRC_PATH)/spi/loggingevent.h \
	$(INCLUDES_SRC_PATH)/spi/objectregistry.h \
	$(INCLUDES_SRC_PATH)/spi/rootlogger.h \
	$(top_builddir)/include/log4cplus/config/defines.hxx

SINGLE_THREADED_SRC = \
    $(INCLUDES_SRC) \
	appenderattachableimpl.cxx \
	appender.cxx \
	configurator.cxx \
	consoleappender.cxx \
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
	loglevel.cxx \
	loglog.cxx \
	logloguser.cxx \
	ndc.cxx \
	nullappender.cxx \
	objectregistry.cxx \
	patternlayout.cxx \
	pointer.cxx \
	property.cxx \
	rootlogger.cxx \
	sleep.cxx \
	socket.cxx \
	socketappender.cxx \
	socketbuffer.cxx \
	stringhelper.cxx \
	syslogappender.cxx \
	timehelper.cxx \
	version.cxx

if WINSOCK_SOCKETS
SOCKETS_SRC = socket-win32.cxx
else
SOCKETS_SRC = socket-unix.cxx
endif

if MULTI_THREADED
THREADS_SRC = threads.cxx \
	syncprims.cxx
else
THREADS_SRC =
endif

LIB_SRC = $(SINGLE_THREADED_SRC) \
	$(THREADS_SRC) \
	$(SOCKETS_SRC)

liblog4cplus_la_SOURCES = $(LIB_SRC)

#liblog4cplus_a_LIBADD = @LIBOBJS@ @ALLOCA@
liblog4cplus_la_LDFLAGS = -no-undefined -version-info @LT_VERSION@ -release 1.0
