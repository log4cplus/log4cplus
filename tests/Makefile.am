SINGLE_THREADED_TESTS = appender_test \
          customloglevel_test \
          fileappender_test \
          filter_test \
          hierarchy_test \
          loglog_test \
          ndc_test \
          ostream_test \
	  patternlayout_test \
	  performance_test \
          priority_test \
	  propertyconfig_test \
	  socket_test \
	  timeformat_test

if MULTI_THREADED
SUBDIRS = $(SINGLE_THREADED_TESTS) thread_test configandwatch_test
else
SUBDIRS = $(SINGLE_THREADED_TESTS)
endif
