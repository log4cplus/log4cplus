cmake_minimum_required (VERSION 2.6)
set (CMAKE_VERBOSE_MAKEFILE on)

find_package (Threads)
message (STATUS "Threads: ${CMAKE_THREAD_LIBS_INIT}")

set (loggingserver_sources
  loggingserver.cxx)

message (STATUS "Sources: ${loggingserver_sources}")

include_directories ("../include")

add_executable (loggingserver ${loggingserver_sources})
target_link_libraries (loggingserver log4cplus)
