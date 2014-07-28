noinst_PROGRAMS += priority_test

priority_test_sources = \
	%D%/main.cxx \
	%D%/func.cxx
priority_test_SOURCES = $(priority_test_sources)

priority_test_LDADD = $(liblog4cplus_la_file)

if BUILD_WITH_WCHAR_T_SUPPORT
noinst_PROGRAMS += priority_testU
priority_testU_CPPFLAGS = $(AM_CPPFLAGS) -DUNICODE=1
priority_testU_SOURCES = $(priority_test_sources)
priority_testU_LDADD = $(liblog4cplusU_la_file)
endif
