

SINGLE_THREADED_TESTS = appender_test \
          fileappender_test \
          filter_test \
          hierarchy_test \
          loglog_test \
          ndc_test \
          ostream_test \
	  patternlayout_test \
          priority_test \
	  propertyconfig_test \
	  shutdown_test \
	  socket_test

if MULTI_THREADED
SUBDIRS = $(SINGLE_THREADED_TESTS) thread_test
else
SUBDIRS = $(SINGLE_THREADED_TESTS)
endif

