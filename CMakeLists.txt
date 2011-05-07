project (log4cplus)
cmake_minimum_required (VERSION 2.6.4)

set (log4cplus_version_major 1)
set (log4cplus_version_minor 1)
set (log4cplus_version_patch 0)
set (log4cplus_soversion 0)
set (log4cplus_postfix "")

find_package (Threads)
message (STATUS "Threads: ${CMAKE_THREAD_LIBS_INIT}")

set(BUILD_SHARED_LIBS TRUE CACHE BOOL "If TRUE, log4cplus is built as a shared library, otherwise as a static library")

option(LOG4CPLUS_WORKING_LOCALE "Define for compilers/standard libraries that support more than just the C locale." OFF)

include(ConfigureChecks.cmake)

configure_file(include/log4cplus/config.h.cmake.in ${CMAKE_BINARY_DIR}/include/log4cplus/config.h @ONLY)
configure_file(include/log4cplus/config/defines.hxx.cmake.in ${CMAKE_BINARY_DIR}/include/log4cplus/config/defines.hxx @ONLY)

include_directories (${CMAKE_SOURCE_DIR}/include
                     ${CMAKE_BINARY_DIR}/include
                    )

enable_testing()

add_subdirectory (src)
add_subdirectory (loggingserver)
add_subdirectory (tests)

include(Log4CPlusCPack.cmake)
