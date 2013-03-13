if (UNICODE)
  add_definitions (-DUNICODE -D_UNICODE -UMBCS -U_MBCS)
endif (UNICODE)

if (${BUILD_SHARED_LIBS})
  add_definitions (-Dlog4cplus_EXPORTS)
endif ()

# a function to set up a test, since it's the same for each one:
function(log4cplus_add_test _name)
  set(_srcs ${ARGN})
#  message (STATUS "${_name} sources: ${_srcs}")
  add_executable (${_name} ${_srcs})
  target_link_libraries (${_name} ${log4cplus})
  add_test(NAME ${_name} WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COMMAND ${_name})
endfunction()


add_subdirectory (appender_test)
add_subdirectory (configandwatch_test)
add_subdirectory (customloglevel_test)
add_subdirectory (fileappender_test)
add_subdirectory (filter_test)
add_subdirectory (hierarchy_test)
add_subdirectory (loglog_test)
add_subdirectory (ndc_test)
add_subdirectory (ostream_test)
add_subdirectory (patternlayout_test)
add_subdirectory (performance_test)
add_subdirectory (priority_test)
add_subdirectory (propertyconfig_test)
#add_subdirectory (socket_test) # I don't know how this test is supposed to be executed
add_subdirectory (thread_test)
add_subdirectory (timeformat_test)
