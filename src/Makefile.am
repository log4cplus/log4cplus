lib_LTLIBRARIES = liblog4cplus.la

AM_CPPFLAGS = -DINSIDE_LOG4CPLUS \
	-I$(top_srcdir)/include -I$(top_builddir)/include \
	@LOG4CPLUS_NDEBUG@

nobase_include_HEADERS = $(top_srcdir)/include/log4cplus/appender.h \
	$(top_srcdir)/include/log4cplus/config.hxx \
	$(top_builddir)/include/log4cplus/config/defines.hxx \
	$(top_srcdir)/include/log4cplus/config/win32.h \
	$(top_srcdir)/include/log4cplus/config/macosx.h \
	$(top_srcdir)/include/log4cplus/configurator.h \
	$(top_srcdir)/include/log4cplus/consoleappender.h \
	$(top_srcdir)/include/log4cplus/fileappender.h \
	$(top_srcdir)/include/log4cplus/fstreams.h \
	$(top_srcdir)/include/log4cplus/hierarchy.h \
	$(top_srcdir)/include/log4cplus/hierarchylocker.h \
	$(top_srcdir)/include/log4cplus/layout.h \
	$(top_srcdir)/include/log4cplus/logger.h \
	$(top_srcdir)/include/log4cplus/loggingmacros.h \
	$(top_srcdir)/include/log4cplus/loglevel.h \
	$(top_srcdir)/include/log4cplus/ndc.h \
	$(top_srcdir)/include/log4cplus/nullappender.h \
	$(top_srcdir)/include/log4cplus/socketappender.h \
	$(top_srcdir)/include/log4cplus/streams.h \
	$(top_srcdir)/include/log4cplus/syslogappender.h \
	$(top_srcdir)/include/log4cplus/tstring.h \
	$(top_srcdir)/include/log4cplus/helpers/appenderattachableimpl.h \
	$(top_srcdir)/include/log4cplus/helpers/loglog.h \
	$(top_srcdir)/include/log4cplus/helpers/logloguser.h \
	$(top_srcdir)/include/log4cplus/helpers/pointer.h \
	$(top_srcdir)/include/log4cplus/helpers/property.h \
	$(top_srcdir)/include/log4cplus/helpers/sleep.h \
	$(top_srcdir)/include/log4cplus/helpers/socketbuffer.h \
	$(top_srcdir)/include/log4cplus/helpers/socket.h \
	$(top_srcdir)/include/log4cplus/helpers/stringhelper.h \
	$(top_srcdir)/include/log4cplus/helpers/thread-config.h \
	$(top_srcdir)/include/log4cplus/helpers/threads.h \
	$(top_srcdir)/include/log4cplus/helpers/timehelper.h \
	$(top_srcdir)/include/log4cplus/spi/appenderattachable.h \
	$(top_srcdir)/include/log4cplus/spi/factory.h \
	$(top_srcdir)/include/log4cplus/spi/filter.h \
	$(top_srcdir)/include/log4cplus/spi/loggerfactory.h \
	$(top_srcdir)/include/log4cplus/spi/loggerimpl.h \
	$(top_srcdir)/include/log4cplus/spi/loggingevent.h \
	$(top_srcdir)/include/log4cplus/spi/objectregistry.h \
	$(top_srcdir)/include/log4cplus/spi/rootlogger.h

SINGLE_THREADED_SRC = appenderattachableimpl.cxx \
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
	socket-unix.cxx \
	socketappender.cxx \
	socketbuffer.cxx \
	stringhelper.cxx \
	syslogappender.cxx \
	timehelper.cxx

if MULTI_THREADED
LIB_SRC = $(SINGLE_THREADED_SRC) threads.cxx
else
LIB_SRC = $(SINGLE_THREADED_SRC)
endif

liblog4cplus_la_SOURCES = $(LIB_SRC) $(nobase_include_HEADERS)

#liblog4cplus_a_LIBADD = @LIBOBJS@ @ALLOCA@
liblog4cplus_la_LDFLAGS = -no-undefined -version-info @LT_VERSION@ -release 1.0
