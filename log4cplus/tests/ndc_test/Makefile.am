

noinst_PROGRAMS = ndc_test

INCLUDES = -I$(top_srcdir)/include

ndc_test_SOURCES = main.cxx

AM_CPPFLAGS = -D_REENTRANT

ndc_test_LDADD = $(top_builddir)/src/liblog4cplus.a -lstdc++ -lpthread

