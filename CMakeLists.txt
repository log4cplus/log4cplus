# This block needs to stay before the project (log4cplus) line so that
#  the output files placed into Android's libs directory.
if (CMAKE_TOOLCHAIN_FILE)
  get_filename_component(TOOL_CHAIN_FILE_NAME "${CMAKE_TOOLCHAIN_FILE}" NAME_WE)
  if("${TOOL_CHAIN_FILE_NAME}" MATCHES "android.toolchain")
    # Android: set output folder to ${CMAKE_BINARY_DIR}
    set (LIBRARY_OUTPUT_PATH_ROOT ${CMAKE_BINARY_DIR} CACHE PATH
      "root for library output, set this to change where Android libs are compiled to")
  endif()
endif ()
# Remove when CMake >= 2.8.4 is required
set (CMAKE_LEGACY_CYGWIN_WIN32 0)

cmake_minimum_required (VERSION 3.12)

project (log4cplus)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.30.0")
  cmake_policy(SET CMP0167 NEW)
endif ()

# Use "-fPIC" / "-fPIE" for all targets by default, including static libs.
set (CMAKE_POSITION_INDEPENDENT_CODE ON)
# Enable LTO / IPO by default.
set (CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
set (CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)
set (CMAKE_INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL ON)

enable_language (CXX)
if (MSVC)
  set (CMAKE_CXX_STANDARD 23)
  if (CMAKE_VS_PLATFORM_NAME MATCHES "ARM")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
  endif ()
else ()
  set (CMAKE_CXX_STANDARD 23)
endif ()
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS ON)
include(GNUInstallDirs)

include (Log4CPlusUtils.cmake)
log4cplus_get_version ("${PROJECT_SOURCE_DIR}/include"
  log4cplus_version_major log4cplus_version_minor log4cplus_version_patch)
set (log4cplus_version_str "${log4cplus_version_major}.${log4cplus_version_minor}.${log4cplus_version_patch}")
message("-- Generating build for Log4cplus version ${log4cplus_version_str}")
set (log4cplus_soversion 0)
set (log4cplus_postfix "")
if (APPLE)
  set (log4cplus_macho_current_version 7.0.0)
  set (log4cplus_macho_compatibility_version 3.0.0)
endif ()

option(LOG4CPLUS_BUILD_TESTING "Build the test suite." ON)
option(LOG4CPLUS_BUILD_LOGGINGSERVER "Build the logging server." ON)

option(LOG4CPLUS_REQUIRE_EXPLICIT_INITIALIZATION "Require explicit initialization (see log4cplus::Initializer)" OFF)
if (LOG4CPLUS_REQUIRE_EXPLICIT_INITIALIZATION)
  add_compile_definitions (LOG4CPLUS_REQUIRE_EXPLICIT_INITIALIZATION=1)
endif(LOG4CPLUS_REQUIRE_EXPLICIT_INITIALIZATION)

option(LOG4CPLUS_ENABLE_THREAD_POOL "Instantiate internal thread pool for when AsyncAppend=true" ON)
if (LOG4CPLUS_ENABLE_THREAD_POOL)
add_compile_definitions (LOG4CPLUS_ENABLE_THREAD_POOL=1)
endif()

if(NOT LOG4CPLUS_SINGLE_THREADED)
  find_package (Threads)
  message (STATUS "Threads: ${CMAKE_THREAD_LIBS_INIT}")
endif(NOT LOG4CPLUS_SINGLE_THREADED)

if(LOG4CPLUS_SINGLE_THREADED AND LOG4CPLUS_BUILD_LOGGINGSERVER)
  message (FATAL_ERROR "The logging server requires the multi-thread capability.")
endif(LOG4CPLUS_SINGLE_THREADED AND LOG4CPLUS_BUILD_LOGGINGSERVER)

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

option(LOG4CPLUS_SINGLE_THREADED
  "Define if you want to build single-threaded version of the library."
  OFF)
if (LOG4CPLUS_SINGLE_THREADED)
  add_compile_definitions (LOG4CPLUS_SINGLE_THREADED=1)
endif (LOG4CPLUS_SINGLE_THREADED)

option (LOG4CPLUS_QT4 "Build with Qt4DebugAppender" OFF)
option (LOG4CPLUS_QT5 "Build with Qt5DebugAppender" OFF)

option(UNICODE "Build with tchar = wchar_t" ${UNICODE_DEFAULT})

option(WITH_ICONV "Use iconv() for char->wchar_t conversion."
  OFF)

option(ENABLE_SYMBOLS_VISIBILITY
  "Enable compiler and platform specific options for symbols visibility"
  ON)

if (MINGW)
  option(LOG4CPLUS_MINGW_STATIC_RUNTIME "Enable MinGW static runtime" OFF)
endif()

option(WITH_UNIT_TESTS "Enable unit tests" ON)
if (WITH_UNIT_TESTS)
  set (LOG4CPLUS_WITH_UNIT_TESTS 1)
  add_compile_definitions (CATCH_CONFIG_PREFIX_ALL=1
    CATCH_AMALGAMATED_CUSTOM_MAIN=1)
  if (WIN32)
    add_compile_definitions (LOG4CPLUS_WITH_UNIT_TESTS=1
      NOMINMAX=1)
    if (MSVC AND MSVC_TOOLSET_VERSION VERSION_LESS "143")
      message (WARNING "Adding /Zc:hiddenFriend- compiler flag to workaround Catch2 complation issue with Visual Studio 2019")
      add_compile_options (/Zc:hiddenFriend-)
    endif ()
  endif (WIN32)
endif (WITH_UNIT_TESTS)

set(_WIN32_WINNT 0x0600 CACHE STRING "Define Windows API version to use.")

option(LOG4CPLUS_ENABLE_DECORATED_LIBRARY_NAME
  "Turns on resulting file name decoration for static and UNICODE builds." ON)
if (LOG4CPLUS_ENABLE_DECORATED_LIBRARY_NAME)
  if (NOT ${BUILD_SHARED_LIBS})
    # set S-prefix for static build
    set (log4cplus_postfix "${log4cplus_postfix}S")
  endif ()

  if (UNICODE)
    set (log4cplus_postfix "${log4cplus_postfix}U")
  endif (UNICODE)
endif (LOG4CPLUS_ENABLE_DECORATED_LIBRARY_NAME)

set (log4cplus "log4cplus${log4cplus_postfix}")

if (WITH_ICONV)
  set(LOG4CPLUS_WITH_ICONV 1)
endif ()

if(LOG4CPLUS_CONFIGURE_CHECKS_PATH)
  get_filename_component(LOG4CPLUS_CONFIGURE_CHECKS_PATH "${LOG4CPLUS_CONFIGURE_CHECKS_PATH}" ABSOLUTE)
endif()

if(LOG4CPLUS_CONFIGURE_CHECKS_PATH AND EXISTS ${LOG4CPLUS_CONFIGURE_CHECKS_PATH})
  message(STATUS "Custom ConfigureChecks script found: ${LOG4CPLUS_CONFIGURE_CHECKS_PATH}")
else()
  set(LOG4CPLUS_CONFIGURE_CHECKS_PATH "ConfigureChecks.cmake")
endif()

include(${LOG4CPLUS_CONFIGURE_CHECKS_PATH})

# Prepare defines.hxx.cmake from defines.hxx.in by doing a bit of
# string manipulation.
file(READ include/log4cplus/config/defines.hxx.in FILE_BUFFER)
string(REGEX REPLACE "#undef[^a-zA-Z0-9_]+([a-zA-Z0-9_]+)"
  "#cmakedefine \\1 @\\1@" FILE_BUFFER "${FILE_BUFFER}")
set(DEFINES_HXX_CMAKE
  ${log4cplus_BINARY_DIR}/include/log4cplus/config/defines.hxx.cmake)
set(DEFINES_HXX ${log4cplus_BINARY_DIR}/include/log4cplus/config/defines.hxx)
file(WRITE ${DEFINES_HXX_CMAKE} ${FILE_BUFFER})

configure_file(${DEFINES_HXX_CMAKE} ${DEFINES_HXX} @ONLY)

include_directories (${log4cplus_SOURCE_DIR}/include
                     ${log4cplus_SOURCE_DIR}/threadpool
                     ${log4cplus_SOURCE_DIR}/catch/extras
                     ${log4cplus_BINARY_DIR}/include
                    )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

if (LOG4CPLUS_BUILD_TESTING)
  enable_testing()
endif (LOG4CPLUS_BUILD_TESTING)

add_subdirectory (src)

# If the CMake version supports it, attach header directory information
# to the targets for when we are part of a parent build (ie being pulled
# in via add_subdirectory() rather than being a standalone build).
if (DEFINED CMAKE_VERSION AND NOT "${CMAKE_VERSION}" VERSION_LESS "2.8.11")
  target_include_directories(${log4cplus} INTERFACE
    $<BUILD_INTERFACE:${log4cplus_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${log4cplus_BINARY_DIR}/include>)
endif()

if (LOG4CPLUS_BUILD_LOGGINGSERVER)
  add_subdirectory (simpleserver)
endif (LOG4CPLUS_BUILD_LOGGINGSERVER)

if (LOG4CPLUS_BUILD_TESTING)
  add_subdirectory (tests)
endif (LOG4CPLUS_BUILD_TESTING)
if (LOG4CPLUS_QT4)
  add_subdirectory (qt4debugappender)
endif (LOG4CPLUS_QT4)
if (LOG4CPLUS_QT5)
  add_subdirectory (qt5debugappender)
endif (LOG4CPLUS_QT5)

include(Log4CPlusCPack.cmake)

# generate pkgconfig file.
set (log4cplus_pkgconfig "${CMAKE_BINARY_DIR}/lib/pkgconfig/log4cplus.pc")
function(log4cplus_gen_pkgconfig)
  message (STATUS "Generating ${log4cplus_pkgconfig}")
  set(prefix "${CMAKE_INSTALL_PREFIX}")
  set(exec_prefix "\${prefix}")
  set(libdir "\${prefix}/${CMAKE_INSTALL_LIBDIR}")
  set(includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
  set(VERSION "${log4cplus_version_major}.${log4cplus_version_minor}.${log4cplus_version_patch}")
  configure_file(log4cplus.pc.in "${log4cplus_pkgconfig}" @ONLY)
endfunction(log4cplus_gen_pkgconfig)
log4cplus_gen_pkgconfig ()

install(FILES "${log4cplus_pkgconfig}" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig/)

message (STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message (STATUS "${CMAKE_CXX_COMPILER_ID} version ${CMAKE_CXX_COMPILER_VERSION}")
message (STATUS "Compiler flags: ${CMAKE_CXX_FLAGS}")
message (STATUS "System name: ${CMAKE_SYSTEM_NAME}")
message (STATUS "System version: ${CMAKE_SYSTEM_VERSION}")
