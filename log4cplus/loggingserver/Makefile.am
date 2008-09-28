AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include

if MULTI_THREADED
noinst_PROGRAMS = loggingserver
loggingserver_SOURCES = loggingserver.cxx
loggingserver_LDADD = $(top_builddir)/src/liblog4cplus.la 
endif
