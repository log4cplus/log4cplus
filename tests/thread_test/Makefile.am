

noinst_PROGRAMS = thread_test

INCLUDES = -I$(top_srcdir)/include

thread_test_SOURCES = main.cxx

AM_CPPFLAGS = -D_REENTRANT

thread_test_LDADD = $(top_builddir)/src/liblog4cplus.a -lstdc++ -lpthread

