set (test_name "customloglevel_test")
set (test_sources
  main.cxx
  func.cxx
  customloglevel.h
  customloglevel.cxx)

project (${test_name} CXX C)
cmake_minimum_required (VERSION 2.6)
set (CMAKE_VERBOSE_MAKEFILE on)

find_package (Threads)

message (STATUS "${test_name} sources: ${test_sources}")

include_directories ("${CMAKE_SOURCE_DIR}/include")
add_executable (${test_name} ${test_sources})
target_link_libraries (${test_name} log4cplus)
