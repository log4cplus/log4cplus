#
# Utility macros for Log4Cplus project
#

# Get Log4cplus version macro
# first param - path to include folder, we will rip version from version.h
macro(log4cplus_get_version _include_PATH vmajor vminor vpatch)
  file(STRINGS "${_include_PATH}/log4cplus/version.h" _log4cplus_VER_STRING_AUX REGEX ".*#define[ ]+LOG4CPLUS_VERSION[ ]+")
  string(REGEX MATCHALL "[0-9]+" _log4clpus_VER_LIST "${_log4cplus_VER_STRING_AUX}")
  list(LENGTH _log4clpus_VER_LIST _log4cplus_VER_LIST_LEN)
# we also count '4' from the name...
  if(_log4cplus_VER_LIST_LEN EQUAL 5)
    list(GET _log4clpus_VER_LIST 2 ${vmajor})
    list(GET _log4clpus_VER_LIST 3 ${vminor})
    list(GET _log4clpus_VER_LIST 4 ${vpatch})
  endif()
endmacro()

function(log4cplus_check_os_log)
  include(CheckCXXSourceCompiles)
  # Link even when a cross toolchain defaults its configure checks to archives.
  set(CMAKE_TRY_COMPILE_TARGET_TYPE EXECUTABLE)
  # The SDK and deployment target can change in an existing build tree.
  unset(LOG4CPLUS_OS_LOG_LINKS CACHE)
  check_cxx_source_compiles([[
    #include <TargetConditionals.h>
    #include <os/log.h>
    #if !TARGET_OS_OSX || __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101200
    #error macOS 10.12 or newer required
    #endif
    int main() {
      os_log_t log = os_log_create("org.log4cplus", "configure");
      if (os_log_type_enabled(log, OS_LOG_TYPE_DEFAULT))
        os_log_with_type(log, OS_LOG_TYPE_DEFAULT, "%{public}s", "probe");
      os_release(log);
    }
  ]] LOG4CPLUS_OS_LOG_LINKS)
  set(LOG4CPLUS_HAVE_OS_LOG ${LOG4CPLUS_OS_LOG_LINKS} PARENT_SCOPE)
endfunction()
