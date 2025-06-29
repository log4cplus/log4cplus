set (qt6debugappender_sources
  qt6debugappender.cxx)

message (STATUS "Configuring Qt6 Debug Appender")
find_package(Qt6 REQUIRED COMPONENTS Core)

set (qt6debugappender log4cplusqt6debugappender${log4cplus_postfix})
add_library (${qt6debugappender} ${qt6debugappender_sources})
add_library (log4cplus::qt6debugappender ALIAS ${qt6debugappender})
if (UNICODE)
  target_compile_definitions (${qt6debugappender} PUBLIC UNICODE)
  target_compile_definitions (${qt6debugappender} PUBLIC _UNICODE)
  add_definitions (-UMBCS -U_MBCS)
endif (UNICODE)
if (${BUILD_SHARED_LIBS})
  target_compile_definitions (${qt6debugappender} PRIVATE ${log4cplus}_EXPORTS)
endif ()
target_link_libraries (${qt6debugappender}
  PUBLIC ${log4cplus}
  PRIVATE Qt6::Core
  PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_include_directories (${qt6debugappender} PUBLIC ${Qt6Core_INCLUDES})
target_compile_definitions (${qt6debugappender} PUBLIC ${Qt6Core_DEFINITIONS})

if (ANDROID)
  # Android does not seem to have SO version support.
elseif (WIN32)
  set_target_properties (${qt6debugappender} PROPERTIES
    VERSION "${log4cplus_version_major}.${log4cplus_version_minor}.${log4cplus_version_patch}")
else ()
  set_target_properties (${qt6debugappender} PROPERTIES
    SOVERSION "${log4cplus_soversion}")
endif ()
target_compile_definitions (${qt6debugappender} PRIVATE INSIDE_LOG4CPLUS_QT5DEBUGAPPENDER)

if (APPLE)
  set_target_properties (${log4cplus} PROPERTIES
    MACHO_CURRENT_VERSION "${log4cplus_macho_current_version}"
    MACHO_COMPATIBILITY_VERSION "${log4cplus_macho_compatibility_version}")
endif ()

if (WIN32)
  set_target_properties (${qt6debugappender} PROPERTIES
    DEBUG_POSTFIX "D")
endif ()

install (TARGETS ${qt6debugappender}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install (FILES
  ../include/log4cplus/qt6debugappender.h
  ../include/log4cplus/qt6messagehandler.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/log4cplus)
