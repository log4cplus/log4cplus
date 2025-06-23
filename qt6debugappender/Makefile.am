lib_LTLIBRARIES += liblog4cplusqt6debugappender.la

liblog4cplusqt6debugappender_la_cppflags = \
	$(AM_CPPFLAGS) \
	-DINSIDE_LOG4CPLUS_QT5DEBUGAPPENDER \
	@QT5_CFLAGS@
liblog4cplusqt6debugappender_la_CPPFLAGS = \
	$(liblog4cplusqt6debugappender_la_cppflags)

liblog4cplusqt6debugappender_la_sources = \
	$(INCLUDES_SRC_PATH)/qt6debugappender.h \
	qt6debugappender/qt6debugappender.cxx
liblog4cplusqt6debugappender_la_SOURCES = \
	$(liblog4cplusqt6debugappender_la_sources)

liblog4cplusqt6debugappender_la_LIBADD = $(liblog4cplus_la_file)

liblog4cplusqt6debugappender_la_ldflags = -no-undefined \
	$(LOG4CPLUS_VERSION_LT_FLAGS) \
	@QT5_LIBS@ @LOG4CPLUS_PROFILING_LDFLAGS@
liblog4cplusqt6debugappender_la_LDFLAGS = \
	$(liblog4cplusqt6debugappender_la_ldflags)

if BUILD_WITH_WCHAR_T_SUPPORT
lib_LTLIBRARIES += liblog4cplusqt6debugappenderU.la
liblog4cplusqt6debugappenderU_la_CPPFLAGS = \
	$(liblog4cplusqt6debugappender_la_cppflags) -DUNICODE=1 -D_UNICODE=1
liblog4cplusqt6debugappenderU_la_SOURCES = \
	$(liblog4cplusqt6debugappender_la_sources)
liblog4cplusqt6debugappenderU_la_LIBADD = $(liblog4cplusU_la_file)
liblog4cplusqt6debugappenderU_la_LDFLAGS = \
	$(liblog4cplusqt6debugappender_la_ldflags)
endif
