AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include \
	@LOG4CPLUS_NDEBUG@
AM_CXXFLAGS=@LOG4CPLUS_PROFILING_CXXFLAGS@
AM_LDFLAGS=@LOG4CPLUS_PROFILING_LDFLAGS@

ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = ChangeLog log4cplus.pc.in
SUBDIRS = include
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = log4cplus.pc

if ENABLE_VERSION_INFO_OPTION
VERSION_INFO_OPTION=-version-info @LT_VERSION@
else
VERSION_INFO_OPTION=-avoid-version
endif

if ENABLE_RELEASE_OPTION
RELEASE_OPTION=-release @LT_RELEASE@
else
RELEASE_OPTION=
endif

LOG4CPLUS_VERSION_LT_FLAGS=$(VERSION_INFO_OPTION) $(RELEASE_OPTION)

INCLUDES_SRC_PATH = $(top_srcdir)/include/log4cplus
liblog4cplus_la_file = $(top_builddir)/liblog4cplus.la
include src/Makefile.am

include tests/appender_test/Makefile.am
include tests/configandwatch_test/Makefile.am
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
include tests/thread_test/Makefile.am

include simpleserver/Makefile.am
include qt4debugappender/Makefile.am
include qt5debugappender/Makefile.am
include swig/Makefile.common.am
include swig/python/Makefile.am

include tests/Makefile.am
