include $(top_srcdir)/tests/Makefile.am.tests

# This is the only program that uses '=' to assign to
# noinst_PROGRAMS. All other test cases and loggingserver use '+='.
noinst_PROGRAMS = appender_test

appender_test_SOURCES = tests/appender_test/main.cxx

appender_test_LDADD = $(liblog4cplus_la_file)

