set (qt4debugappender_sources
  qt4debugappender.cxx)

find_package (Qt4 REQUIRED)
include (${QT_USE_FILE})

set (qt4debugappender log4cplusqt4debugappender${log4cplus_postfix})
add_library (${qt4debugappender} ${qt4debugappender_sources})
if (UNICODE)
  target_compile_definitions (${qt4debugappender} PUBLIC UNICODE)
  target_compile_definitions (${qt4debugappender} PUBLIC _UNICODE)
  add_definitions (-UMBCS -U_MBCS)
endif (UNICODE)
if (${BUILD_SHARED_LIBS})
  target_compile_definitions (${qt4debugappender} PRIVATE ${log4cplus}_EXPORTS)
endif ()
target_link_libraries (${qt4debugappender}
  ${log4cplus}
  ${QT_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT})

if (ANDROID)
  # Android does not seem to have SO version support.
elseif (WIN32)
  set_target_properties (${qt4debugappender} PROPERTIES
    VERSION "${log4cplus_version_major}.${log4cplus_version_minor}.${log4cplus_version_patch}")
else ()
  set_target_properties (${qt4debugappender} PROPERTIES
    SOVERSION "${log4cplus_soversion}")
endif ()
target_compile_definitions (${qt4debugappender} PRIVATE INSIDE_LOG4CPLUS_QT4DEBUGAPPENDER)

if (APPLE)
  set_target_properties (${log4cplus} PROPERTIES
    MACHO_CURRENT_VERSION "${log4cplus_macho_current_version}"
    MACHO_COMPATIBILITY_VERSION "${log4cplus_macho_compatibility_version}")
endif ()

if (WIN32)
  set_target_properties (${qt4debugappender} PROPERTIES
    DEBUG_POSTFIX "D")
endif ()

install (TARGETS ${qt4debugappender}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install (FILES ../include/log4cplus/qt4debugappender.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/log4cplus)
