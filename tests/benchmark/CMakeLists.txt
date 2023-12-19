#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

option(BUILD_BENCHMARK_CHECKS "Build google benchmark checks"  OFF)

if( NOT BUILD_BENCHMARK_CHECKS )
	return()
endif()
find_package(benchmark REQUIRED)

add_executable(gbenchmarks benchmark.cpp)

# Note: we need to include the DLLs on our path so that the tests will run.
# The way that CMake sets the environment is that it actually generates a secondary file,
# CTestTestfile.cmake, which sets the final properties of the test.
# However, this results in a secondary quirk to the running of the tests: CMake uses
# a semicolon to deliminate entries in a list!  Since the Windows PATH is semicolon-delimited
# as well, CMake uses only the first entry in the list when setting the path.
# So, we need to do a triple escape on the PATH that we want to set in order for CMake to
# properly interpret the PATH
if( WIN32 )
  set(LOG4CPLUS_DLL_DIR "$<SHELL_PATH:$<TARGET_FILE_DIR:log4cplus::log4cplus>>;")
  set(PATH_FOR_TESTS ${CMAKE_PROGRAM_PATH};${LOG4CPLUS_DLL_DIR}\;)
  list(REMOVE_DUPLICATES PATH_FOR_TESTS)
  set(NORMAL_PATH $ENV{PATH})
  set(ESCAPED_PATH "")
  foreach( ENTRY ${PATH_FOR_TESTS}${NORMAL_PATH} )
	  set(ESCAPED_PATH "${ESCAPED_PATH}${ENTRY}\\\;")
  endforeach()
  set_target_properties(gbenchmarks PROPERTIES
    VS_DEBUGGER_ENVIRONMENT "LOG4CPLUS_BENCHMARK_THREAD_COUNT=4\nPATH=${ESCAPED_PATH}"
  )
else()
add_custom_target(run-gbenchmarks COMMAND gbenchmarks DEPENDS gbenchmarks)
endif( WIN32 )

target_compile_definitions(gbenchmarks PRIVATE ${LOG4CPLUS_COMPILE_DEFINITIONS})
target_include_directories(gbenchmarks PRIVATE $<TARGET_PROPERTY:log4cplus::log4cplus,INCLUDE_DIRECTORIES>)
target_link_libraries(gbenchmarks PRIVATE log4cplus::log4cplus benchmark::benchmark benchmark::benchmark_main)
