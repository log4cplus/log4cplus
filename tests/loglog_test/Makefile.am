## Generated by Autogen from Makefile.am.tpl

noinst_PROGRAMS += loglog_test

loglog_test_sources = \
	%D%/main.cxx

loglog_test_SOURCES = $(loglog_test_sources)

loglog_test_CPPFLAGS = $(AM_CPPFLAGS)
loglog_test_LDADD = $(liblog4cplus_la_file)
loglog_test_LDFLAGS = -no-install

if BUILD_WITH_WCHAR_T_SUPPORT
noinst_PROGRAMS += loglog_testU
loglog_testU_CPPFLAGS = $(AM_CPPFLAGS) -DUNICODE=1 -D_UNICODE=1
loglog_testU_SOURCES = $(loglog_test_sources)
loglog_testU_LDADD = $(liblog4cplusU_la_file)
loglog_testU_LDFLAGS = -no-install
endif # BUILD_WITH_WCHAR_T_SUPPORT
