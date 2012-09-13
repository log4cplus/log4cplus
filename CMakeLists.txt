project (log4cplus)
cmake_minimum_required (VERSION 2.8.0)

set (log4cplus_version_major 1)
set (log4cplus_version_minor 1)
set (log4cplus_version_patch 0)
set (log4cplus_soversion 0)
set (log4cplus_postfix "")

option(LOG4CPLUS_BUILD_TESTING "Build the test suite." ON)

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

option(LOG4CPLUS_WORKING_C_LOCALE
  "Define for compilers/standard libraries that have working C locale."
  OFF)

option (LOG4CPLUS_QT4 "Build with Qt4DebugAppender" OFF)

option(UNICODE "Build with tchar = wchar_t" ${UNICODE_DEFAULT})

option(WITH_ICONV "Use iconv() for char->wchar_t conversion."
  OFF)

if (UNICODE)
  set (log4cplus_postfix "${log4cplus_postfix}U")
endif (UNICODE)

if (WITH_ICONV)
  set(LOG4CPLUS_WITH_ICONV 1)
endif ()

set (log4cplus "log4cplus${log4cplus_postfix}")

include(ConfigureChecks.cmake)

configure_file(include/log4cplus/config.h.cmake.in
  ${log4cplus_BINARY_DIR}/include/log4cplus/config.h @ONLY)
configure_file(include/log4cplus/config/defines.hxx.cmake.in
  ${log4cplus_BINARY_DIR}/include/log4cplus/config/defines.hxx @ONLY)

include_directories (${log4cplus_SOURCE_DIR}/include
                     ${log4cplus_BINARY_DIR}/include
                    )

if (LOG4CPLUS_BUILD_TESTING)
  enable_testing()
endif (LOG4CPLUS_BUILD_TESTING)

add_subdirectory (src)
add_subdirectory (loggingserver)
if (LOG4CPLUS_BUILD_TESTING)
  add_subdirectory (tests)
endif (LOG4CPLUS_BUILD_TESTING)
if (LOG4CPLUS_QT4)
  add_subdirectory (qt4debugappender)
endif (LOG4CPLUS_QT4)

include(Log4CPlusCPack.cmake)
