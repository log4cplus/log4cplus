
if MULTI_THREADED
noinst_PROGRAMS = loggingserver
loggingserver_SOURCES = loggingserver.cxx
loggingserver_LDADD = $(top_builddir)/src/liblog4cplus.la 
endif

lib_LTLIBRARIES = liblog4cplus.la

AM_CPPFLAGS = -Wall

INCLUDES = -I../include 

SINGLE_THREADED_SRC = appenderattachableimpl.cxx \
                         appender.cxx \
                         configurator.cxx \
                         consoleappender.cxx \
			 factory.cxx \
                         fileappender.cxx \
                         filter.cxx \
			 global-init.cxx \
                         hierarchy.cxx \
                         layout.cxx \
                         logger.cxx \
                         loggerimpl.cxx \
                         loglevel.cxx \
                         loglog.cxx \
                         ndc.cxx \
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

