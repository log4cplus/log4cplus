
noinst_PROGRAMS = loggingserver
loggingserver_SOURCES = loggingserver.cxx
loggingserver_LDADD = $(top_builddir)/src/liblog4cplus.la -lstdc++ -lpthread


lib_LTLIBRARIES = liblog4cplus.la

AM_CPPFLAGS = -D_REENTRANT

INCLUDES = -I../include 

liblog4cplus_la_SOURCES = appenderattachableimpl.cxx \
                         appender.cxx \
                         configurator.cxx \
                         consoleappender.cxx \
			 factory.cxx \
                         fileappender.cxx \
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
			 socket.cxx \
			 socket-unix.cxx \
			 socketappender.cxx \
			 socketbuffer.cxx \
			 stringhelper.cxx \
			 strftime.cxx \
			 syslogappender.cxx \
                         threads.cxx

#liblog4cplus_a_LIBADD = @LIBOBJS@ @ALLOCA@
liblog4cplus_la_LDFLAGS = -version-info @LT_VERSION@

