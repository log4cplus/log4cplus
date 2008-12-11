project (log4cplus)
cmake_minimum_required (VERSION 2.6)
set (CMAKE_VERBOSE_MAKEFILE on)

set (log4cplus_version_major 1)
set (log4cplus_version_minor 0)
set (log4cplus_version_patch 3)
set (log4cplus_soversion 3)

find_package (Threads)
message (STATUS "Threads: ${CMAKE_THREAD_LIBS_INIT}")

set(log4cplus_headers
  include/log4cplus/appender.h
  include/log4cplus/config/macosx.h
  include/log4cplus/config/win32.h
  include/log4cplus/config.hxx
  include/log4cplus/configurator.h
  include/log4cplus/consoleappender.h
  include/log4cplus/fileappender.h
  include/log4cplus/fstreams.h
  include/log4cplus/helpers/appenderattachableimpl.h
  include/log4cplus/helpers/loglog.h
  include/log4cplus/helpers/logloguser.h
  include/log4cplus/helpers/pointer.h
  include/log4cplus/helpers/property.h
  include/log4cplus/helpers/sleep.h
  include/log4cplus/helpers/socket.h
  include/log4cplus/helpers/socketbuffer.h
  include/log4cplus/helpers/stringhelper.h
  include/log4cplus/helpers/thread-config.h
  include/log4cplus/helpers/threads.h
  include/log4cplus/helpers/timehelper.h
  include/log4cplus/hierarchy.h
  include/log4cplus/hierarchylocker.h
  include/log4cplus/layout.h
  include/log4cplus/logger.h
  include/log4cplus/loggingmacros.h
  include/log4cplus/loglevel.h
  include/log4cplus/ndc.h
  include/log4cplus/nteventlogappender.h
  include/log4cplus/nullappender.h
  include/log4cplus/socketappender.h
  include/log4cplus/spi/appenderattachable.h
  include/log4cplus/spi/factory.h
  include/log4cplus/spi/filter.h
  include/log4cplus/spi/loggerfactory.h
  include/log4cplus/spi/loggerimpl.h
  include/log4cplus/spi/loggingevent.h
  include/log4cplus/spi/objectregistry.h
  include/log4cplus/spi/rootlogger.h
  include/log4cplus/stamp-h1
  include/log4cplus/streams.h
  include/log4cplus/syslogappender.h
  include/log4cplus/tstring.h
  include/log4cplus/win32debugappender.h)
message (STATUS "Headers: ${log4cplus_headers}")


set (log4cplus_sources
  src/appender.cxx
  src/appenderattachableimpl.cxx
  src/configurator.cxx
  src/consoleappender.cxx
  src/factory.cxx
  src/fileappender.cxx
  src/filter.cxx
  src/global-init.cxx
  src/hierarchy.cxx
  src/hierarchylocker.cxx
  src/layout.cxx
  src/logger.cxx
  src/loggerimpl.cxx
  src/loggingevent.cxx
  src/loglevel.cxx
  src/loglog.cxx
  src/logloguser.cxx
  src/ndc.cxx
  src/nullappender.cxx
  src/objectregistry.cxx
  src/patternlayout.cxx
  src/pointer.cxx
  src/property.cxx
  src/rootlogger.cxx
  src/sleep.cxx
  src/socket.cxx
  src/socketappender.cxx
  src/socketbuffer.cxx
  src/stringhelper.cxx
  src/syslogappender.cxx
  src/threads.cxx
  src/timehelper.cxx)
message (STATUS "Sources: ${log4cplus_sources}")

#message (STATUS "Type: ${UNIX}|${CYGWIN}|${WIN32}")

if (${UNIX} OR ${CYGWIN})
  set (log4cplus_sources ${log4cplus_sources}
    src/socket-unix.cxx)
  set (log4cplus_headers ${log4cplus_headers}
    include/log4cplus/config/defines.hxx)
elseif (WIN32)
  set (log4cplus_sources ${log4cplus_sources}
    src/socket-win32.cxx
    src/win32debugappender.cxx
    src/nteventlogappender.cxx)
  
  #add_definitions (-DLOG4CPLUS_STATIC)
  add_definitions (-DLOG4CPLUS_BUILD_DLL)
 
  # Comment out the following line if you want non-Unicode version.
  add_definitions (-DUNICODE -D_UNICODE -UMBCS -U_MBCS)
endif ()

include_directories ("include")
add_definitions (-DINSIDE_LOG4CPLUS)
set (log4cplus_all_sources ${log4cplus_sources} ${log4cplus_headers})

#add_library (log4cplus STATIC ${log4cplus_all_sources})
add_library (log4cplus SHARED ${log4cplus_all_sources})

target_link_libraries (log4cplus ${CMAKE_THREAD_LIBS_INIT})
set_target_properties (log4cplus PROPERTIES
  VERSION "${log4cplus_version_major}.${log4cplus_version_minor}"
  SOVERSION "${log4cplus_soversion}")
