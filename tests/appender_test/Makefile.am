

noinst_PROGRAMS = appender_test

INCLUDES = -I$(top_srcdir)/include

appender_test_SOURCES = main.cxx

AM_CPPFLAGS = -D_REENTRANT

appender_test_LDADD = $(top_builddir)/src/liblog4cplus.la -lstdc++ -lpthread

