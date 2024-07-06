set (unit_tests_sources unit_tests.cxx)
set (unit_tests_libs ${log4cplus})


add_executable (unit_tests ${unit_tests_sources})
target_link_libraries (unit_tests PUBLIC ${unit_tests_libs})
add_test (NAME unit_tests
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND unit_tests)
