AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include \
	@LOG4CPLUS_NDEBUG@

if MULTI_THREADED
noinst_PROGRAMS = loggingserver
loggingserver_SOURCES = loggingserver.cxx
loggingserver_LDADD = $(top_builddir)/src/liblog4cplus.la 
endif
