lib_LTLIBRARIES += liblog4cplus-opentelemetry.la
liblog4cplus_opentelemetry_la_LIBADD = $(liblog4cplus_la_file) $(OPENTELEMETRY_LIBS)

liblog4cplus_opentelemetry_la_cppflags = \
	$(AM_CPPFLAGS) \
	-DINSIDE_LOG4CPLUS_OPENTELEMETRYAPPENDER \
	$(OPENTELEMETRY_CPPFLAGS)
liblog4cplus_opentelemetry_la_CPPFLAGS = \
	$(liblog4cplus_opentelemetry_la_cppflags)

liblog4cplus_opentelemetry_la_sources = \
	%D%/opentelemetryappender.cxx \
	$(INCLUDES_SRC_PATH)/opentelemetryappender.h
liblog4cplus_opentelemetry_la_SOURCES = \
	$(liblog4cplus_opentelemetry_la_sources)

liblog4cplus_opentelemetry_la_ldflags = -no-undefined \
	$(LOG4CPLUS_VERSION_LT_FLAGS) \
	@LOG4CPLUS_PROFILING_LDFLAGS@
liblog4cplus_opentelemetry_la_LDFLAGS = \
	$(liblog4cplus_opentelemetry_la_ldflags)

if BUILD_WITH_WCHAR_T_SUPPORT
lib_LTLIBRARIES += liblog4cplus-opentelemetryU.la
liblog4cplus_opentelemetryU_la_LIBADD = $(liblog4cplusU_la_file) $(OPENTELEMETRY_LIBS)
liblog4cplus_opentelemetryU_la_CPPFLAGS = \
	$(liblog4cplus_opentelemetry_la_cppflags) -DUNICODE=1 -D_UNICODE=1
liblog4cplus_opentelemetryU_la_SOURCES = \
	$(liblog4cplus_opentelemetry_la_sources)
liblog4cplus_opentelemetryU_la_LDFLAGS = \
	$(liblog4cplus_opentelemetry_la_ldflags)
endif
