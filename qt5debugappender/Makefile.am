if QT5
lib_LTLIBRARIES += liblog4cplusqt5debugappender.la

liblog4cplusqt5debugappender_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-DINSIDE_LOG4CPLUS_QT5DEBUGAPPENDER \
	@QT5_CFLAGS@

#liblog4cplusqt5debugappender_la_CXXFLAGS=-no-suppress \
#	 @LOG4CPLUS_PROFILING_CXXFLAGS@

liblog4cplusqt5debugappender_la_SOURCES = \
	$(INCLUDES_SRC_PATH)/qt5debugappender.h \
	qt5debugappender/qt5debugappender.cxx

liblog4cplusqt5debugappender_la_LIBADD = $(liblog4cplus_la_file)

liblog4cplusqt5debugappender_la_LDFLAGS = -no-undefined \
	$(LOG4CPLUS_VERSION_LT_FLAGS) \
	@QT5_LIBS@ @LOG4CPLUS_PROFILING_LDFLAGS@
endif
