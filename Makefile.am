ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = ChangeLog log4cplus.pc.in
SUBDIRS = include
if WITH_SWIG
SUBDIRS += swig
endif
if QT
SUBDIRS += qt4debugappender
endif
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = log4cplus.pc

liblog4cplus_la_file = $(top_builddir)/liblog4cplus.la
include src/Makefile.am

include tests/appender_test/Makefile.am
include tests/customloglevel_test/Makefile.am
include tests/fileappender_test/Makefile.am
include tests/filter_test/Makefile.am
include tests/hierarchy_test/Makefile.am
include tests/loglog_test/Makefile.am
include tests/ndc_test/Makefile.am
include tests/ostream_test/Makefile.am
include tests/patternlayout_test/Makefile.am
include tests/performance_test/Makefile.am
include tests/priority_test/Makefile.am
include tests/propertyconfig_test/Makefile.am
include tests/socket_test/Makefile.am
include tests/timeformat_test/Makefile.am

if MULTI_THREADED
include tests/thread_test/Makefile.am
include tests/configandwatch_test/Makefile.am
endif

include simpleserver/Makefile.am

TESTSUITE = tests/testsuite
AUTOTEST = $(AUTOM4TE) --language=Autotest
TESTSUITE_AT = \
  tests/appender_test.at \
  tests/configandwatch_test.at \
  tests/customloglevel_test.at \
  tests/fileappender_test.at \
  tests/filter_test.at \
  tests/hierarchy_test.at \
  tests/loglog_test.at \
  tests/ndc_test.at \
  tests/ostream_test.at \
  tests/patternlayout_test.at \
  tests/priority_test.at \
  tests/propertyconfig_test.at \
  tests/thread_test.at \
  tests/timeformat_test.at \
  tests/local.at \
  tests/testsuite.at

$(TESTSUITE): $(TESTSUITE_AT)
	$(AUTOTEST) -I $(srcdir) $(srcdir)/tests/testsuite.at -o $@

tests/atconfig: $(top_builddir)/config.status
	cd "$(top_builddir)" && ./config.status tests/$@

check-local: tests/atconfig tests/atlocal $(TESTSUITE)
	cd "$(top_builddir)/tests" && $(SHELL) $(abs_top_srcdir)/$(TESTSUITE) $(TESTSUITEFLAGS)

clean-local:
	cd "$(top_builddir)/tests" && (test ! -f '$(abs_top_srcdir)/$(TESTSUITE)' || $(SHELL) '$(abs_top_srcdir)/$(TESTSUITE)' --clean)

EXTRA_DIST += tests/testsuite.at $(TESTSUITE) tests/atlocal.in
