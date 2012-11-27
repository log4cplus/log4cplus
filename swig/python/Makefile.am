if WITH_PYTHON
PYTHON_WRAP_CXX = python_wrap.cxx
BUILT_SOURCES = $(PYTHON_WRAP_CXX)

pkgpython_PYTHON = log4cplus.py
pkgpyexec_LTLIBRARIES = _log4cplus.la
_log4cplus_la_SOURCES = $(PYTHON_WRAP_CXX) $(SWIG_SOURCES)
_log4cplus_la_CPPFLAGS = $(AM_CPPFLAGS) $(SWIG_PYTHON_CPPFLAGS) \
  $(PYTHON_CPPFLAGS) -I$(top_srcdir)/include -I$(top_builddir)/include \
  -DSWIG_TYPE_TABLE=log4cplus
_log4cplus_la_LDFLAGS = -no-undefined -shared -module -avoid-version \
  $(PYTHON_LDFLAGS) $(AM_LDFLAGS)
_log4cplus_la_LIBADD = $(liblog4cplus_la_file)

$(PYTHON_WRAP_CXX): $(SWIG_SOURCES)
	$(SWIG) $(SWIG_FLAGS) -c++ -python $(SWIG_PYTHON_OPT) \
	-I$(top_srcdir)/include -I$(top_builddir)/include \
	-I$(top_srcdir)/swig -o $(PYTHON_WRAP_CXX) \
	$(top_srcdir)/swig/log4cplus.swg

endif
