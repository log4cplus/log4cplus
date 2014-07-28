noinst_PROGRAMS += performance_test

performance_test_sources = %D%/main.cxx
performance_test_SOURCES = $(performance_test_sources)

performance_test_LDADD = $(liblog4cplus_la_file)

if BUILD_WITH_WCHAR_T_SUPPORT
noinst_PROGRAMS += performance_testU
performance_testU_CPPFLAGS = $(AM_CPPFLAGS) -DUNICODE=1
performance_testU_SOURCES = $(performance_test_sources)
performance_testU_LDADD = $(liblog4cplusU_la_file)
endif
