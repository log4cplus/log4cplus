

lib_LTLIBRARIES = liblog4cplus.la

AM_CPPFLAGS = -D_REENTRANT

INCLUDES = -I../include 

liblog4cplus_la_SOURCES = appenderattachableimpl.cxx \
                         appender.cxx \
                         configurator.cxx \
                         consoleappender.cxx \
			 factory.cxx \
                         fileappender.cxx \
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
			 stringhelper.cxx \
			 syslogappender.cxx \
                         threads.cxx

#liblog4cplus_a_LIBADD = @LIBOBJS@ @ALLOCA@
liblog4cplus_la_LDFLAGS = -version-info @LT_VERSION@

