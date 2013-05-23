lib_LTLIBRARIES = liblog4cplusqt4debugappender.la

AM_CPPFLAGS = -DINSIDE_LOG4CPLUS_QT4DEBUGAPPENDER \
	-I$(top_srcdir)/include -I$(top_builddir)/include \
	@LOG4CPLUS_NDEBUG@ @QT_CFLAGS@

AM_CXXFLAGS=-no-suppress @LOG4CPLUS_PROFILING_CXXFLAGS@

AM_LDFLAGS=@LOG4CPLUS_PROFILING_LDFLAGS@

INCLUDES_SRC_PATH = $(top_srcdir)/include/log4cplus

INCLUDES_SRC = \
	$(INCLUDES_SRC_PATH)/qt4debugappender.h

LIB_SRC = $(INCLUDES_SRC) \
	qt4debugappender.cxx

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

liblog4cplusqt4debugappender_la_SOURCES = $(LIB_SRC)

liblog4cplusqt4debugappender_la_LIBADD = $(top_builddir)/src/liblog4cplus.la

liblog4cplusqt4debugappender_la_LDFLAGS = -no-undefined \
	$(LOG4CPLUS_VERSION_LT_FLAGS) \
	@QT_LIBS@
