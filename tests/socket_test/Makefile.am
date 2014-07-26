noinst_PROGRAMS += socket_test

socket_test_sources = %D%/main.cxx
socket_test_SOURCES = $(socket_test_sources)

socket_test_LDADD = $(liblog4cplus_la_file)

if BUILD_WITH_WCHAR_T_SUPPORT
noinst_PROGRAMS += socket_testU
socket_testU_CPPFLAGS = $(AM_CPPFLAGS) -DUNICODE=1
socket_testU_SOURCES = $(socket_test_sources)
socket_testU_LDADD = $(liblog4cplusU_la_file)
endif
