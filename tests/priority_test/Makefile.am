

noinst_PROGRAMS = priority_test

INCLUDES = -I$(top_srcdir)/include

priority_test_SOURCES = main.cxx func.cxx

AM_CPPFLAGS = -D_REENTRANT

priority_test_LDADD = $(top_builddir)/src/liblog4cplus.a -lstdc++ -lpthread

