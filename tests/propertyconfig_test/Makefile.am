noinst_PROGRAMS += propertyconfig_test

propertyconfig_test_sources = %D%/main.cxx
propertyconfig_test_SOURCES = $(propertyconfig_test_sources)

propertyconfig_test_LDADD = $(liblog4cplus_la_file)

if BUILD_WITH_WCHAR_T_SUPPORT
noinst_PROGRAMS += propertyconfig_testU
propertyconfig_testU_CPPFLAGS = $(AM_CPPFLAGS) -DUNICODE=1
propertyconfig_testU_SOURCES = $(propertyconfig_test_sources)
propertyconfig_testU_LDADD = $(liblog4cplusU_la_file)
endif
