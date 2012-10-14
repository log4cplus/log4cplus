include $(srcdir)/../Makefile.common.am

WRAPPER_CXX = $(builddir)/python_wrap.cxx
BUILT_SOURCES = $(WRAPPER_CXX)

pkgpython_PYTHON = log4cplus.py
pkgpyexec_LTLIBRARIES = _log4cplus.la
_log4cplus_la_SOURCES = $(WRAPPER_CXX) $(SWIG_SOURCES)
_log4cplus_la_CPPFLAGS = $(SWIG_PYTHON_CPPFLAGS) $(PYTHON_CPPFLAGS) \
  -I$(top_srcdir)/include -I$(top_builddir)/include \
  -DSWIG_TYPE_TABLE=log4cplus
_log4cplus_la_LDFLAGS = -no-undefined -shared -module -avoid-version \
  $(PYTHON_LDFLAGS)
_log4cplus_la_LIBADD = $(top_builddir)/src/liblog4cplus.la

$(WRAPPER_CXX): $(SWIG_SOURCES)
	$(SWIG) $(SWIG_FLAGS) -c++ -python $(SWIG_PYTHON_OPT) \
	-I$(top_srcdir)/include -I$(top_builddir)/include \
	-I$(top_srcdir)/swig -o $(WRAPPER_CXX) \
	$(srcdir)/../log4cplus.swg
