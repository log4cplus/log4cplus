

noinst_PROGRAMS = loglog_test

INCLUDES = -I$(top_srcdir)/include

loglog_test_SOURCES = main.cxx

AM_CPPFLAGS = -D_REENTRANT

loglog_test_LDADD = $(top_builddir)/src/liblog4cplus.la -lstdc++ -lpthread

