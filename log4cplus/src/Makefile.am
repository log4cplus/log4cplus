
if MULTI_THREADED
noinst_PROGRAMS = loggingserver
loggingserver_SOURCES = loggingserver.cxx
loggingserver_LDADD = $(top_builddir)/src/liblog4cplus.la 
endif

lib_LTLIBRARIES = liblog4cplus.la

AM_CPPFLAGS = -Wall

INCLUDES = -I../include 

nobase_include_HEADERS = ../include/log4cplus/appender.h \
			 ../include/log4cplus/config.h \
			 ../include/log4cplus/configurator.h \
			 ../include/log4cplus/consoleappender.h \
			 ../include/log4cplus/fileappender.h \
			 ../include/log4cplus/fstreams.h \
			 ../include/log4cplus/hierarchy.h \
			 ../include/log4cplus/hierarchylocker.h \
			 ../include/log4cplus/layout.h \
			 ../include/log4cplus/logger.h \
			 ../include/log4cplus/loggingmacros.h \
			 ../include/log4cplus/loglevel.h \
			 ../include/log4cplus/ndc.h \
			 ../include/log4cplus/nullappender.h \
			 ../include/log4cplus/socketappender.h \
			 ../include/log4cplus/streams.h \
			 ../include/log4cplus/syslogappender.h \
			 ../include/log4cplus/tstring.h \
			 ../include/log4cplus/helpers/appenderattachableimpl.h \
			 ../include/log4cplus/helpers/loglog.h \
			 ../include/log4cplus/helpers/logloguser.h \
			 ../include/log4cplus/helpers/pointer.h \
			 ../include/log4cplus/helpers/property.h \
			 ../include/log4cplus/helpers/sleep.h \
			 ../include/log4cplus/helpers/socketbuffer.h \
			 ../include/log4cplus/helpers/socket.h \
			 ../include/log4cplus/helpers/stringhelper.h \
			 ../include/log4cplus/helpers/thread-config.h \
			 ../include/log4cplus/helpers/threads.h \
			 ../include/log4cplus/helpers/timehelper.h \
			 ../include/log4cplus/spi/appenderattachable.h \
			 ../include/log4cplus/spi/factory.h \
			 ../include/log4cplus/spi/filter.h \
			 ../include/log4cplus/spi/loggerfactory.h \
			 ../include/log4cplus/spi/loggerimpl.h \
			 ../include/log4cplus/spi/loggingevent.h \
			 ../include/log4cplus/spi/objectregistry.h \
			 ../include/log4cplus/spi/rootlogger.h

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

liblog4cplus_la_SOURCES = $(LIB_SRC)

#liblog4cplus_a_LIBADD = @LIBOBJS@ @ALLOCA@
liblog4cplus_la_LDFLAGS = -version-info @LT_VERSION@

