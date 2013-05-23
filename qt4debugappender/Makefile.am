if QT
lib_LTLIBRARIES += liblog4cplusqt4debugappender.la

liblog4cplusqt4debugappender_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-DINSIDE_LOG4CPLUS_QT4DEBUGAPPENDER \
	@QT_CFLAGS@

#liblog4cplusqt4debugappender_la_CXXFLAGS=-no-suppress \
#	 @LOG4CPLUS_PROFILING_CXXFLAGS@

liblog4cplusqt4debugappender_la_SOURCES = \
	$(INCLUDES_SRC_PATH)/qt4debugappender.h \
	qt4debugappender/qt4debugappender.cxx

liblog4cplusqt4debugappender_la_LIBADD = $(liblog4cplus_la_file)

liblog4cplusqt4debugappender_la_LDFLAGS = -no-undefined \
	$(LOG4CPLUS_VERSION_LT_FLAGS) \
	@QT_LIBS@ @LOG4CPLUS_PROFILING_LDFLAGS@
endif
