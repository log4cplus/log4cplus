if MULTI_THREADED
noinst_PROGRAMS += loggingserver
loggingserver_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include \
	@LOG4CPLUS_NDEBUG@
loggingserver_SOURCES = simpleserver/loggingserver.cxx
loggingserver_LDADD = $(liblog4cplus_la_file)
endif
