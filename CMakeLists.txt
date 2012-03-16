project (log4cplus)
cmake_minimum_required (VERSION 2.8.0)

set (log4cplus_version_major 1)
set (log4cplus_version_minor 1)
set (log4cplus_version_patch 0)
set (log4cplus_soversion 0)
set (log4cplus_postfix "")

find_package (Threads)
message (STATUS "Threads: ${CMAKE_THREAD_LIBS_INIT}")

set(BUILD_SHARED_LIBS TRUE CACHE BOOL "If TRUE, log4cplus is built as a shared library, otherwise as a static library")

if (WIN32)
  set (UNICODE_DEFAULT ON)
else (WIN32)
  set (UNICODE_DEFAULT OFF)
endif (WIN32)

if (MSVC)
  set (LOG4CPLUS_WORKING_LOCALE_DEFAULT ON)
else (MSVC)
  set (LOG4CPLUS_WORKING_LOCALE_DEFAULT OFF)
endif (MSVC)

option(LOG4CPLUS_WORKING_LOCALE "Define for compilers/standard libraries that support more than just the C locale."
  ${LOG4CPLUS_WORKING_LOCALE_DEFAULT})

option(LOG4CPLUS_WORKING_C_LOCALE "Define for compilers/standard libraries that have working C locale."
  OFF)

option(UNICODE "Build with tchar = wchar_t" ${UNICODE_DEFAULT})

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
