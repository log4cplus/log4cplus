

noinst_PROGRAMS = fileappender_test

INCLUDES = -I$(top_srcdir)/include

fileappender_test_SOURCES = main.cxx

AM_CPPFLAGS = -D_REENTRANT

fileappender_test_LDADD = $(top_builddir)/src/liblog4cplus.a -lstdc++ -lpthread

