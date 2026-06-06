set (opentelemetryappender_sources
  opentelemetryappender.cxx)

message (STATUS "Configuring OpenTelemetry Appender")
find_package (opentelemetry-cpp CONFIG REQUIRED COMPONENTS api)

if (LOG4CPLUS_OPENTELEMETRY_ENABLE_OTLP)
  find_package (opentelemetry-cpp CONFIG REQUIRED COMPONENTS sdk)
  set (LOG4CPLUS_WITH_OPENTELEMETRY_OTLP 1)
  find_package (opentelemetry-cpp CONFIG COMPONENTS otlp_http_log_record_exporter)
  if (TARGET opentelemetry-cpp::otlp_http_log_record_exporter)
    set (LOG4CPLUS_WITH_OPENTELEMETRY_OTLP_HTTP 1)
  endif ()
  find_package (opentelemetry-cpp CONFIG COMPONENTS otlp_grpc_log_record_exporter)
  if (TARGET opentelemetry-cpp::otlp_grpc_log_record_exporter)
    set (LOG4CPLUS_WITH_OPENTELEMETRY_OTLP_GRPC 1)
  endif ()
  if (NOT LOG4CPLUS_WITH_OPENTELEMETRY_OTLP_HTTP
      AND NOT LOG4CPLUS_WITH_OPENTELEMETRY_OTLP_GRPC)
    message (FATAL_ERROR "LOG4CPLUS_OPENTELEMETRY_ENABLE_OTLP requires an OpenTelemetry C++ OTLP log record exporter target")
  endif ()
endif ()

set (opentelemetryappender log4cplus-opentelemetry${log4cplus_postfix})
add_library (${opentelemetryappender} ${opentelemetryappender_sources})
add_library (log4cplus::opentelemetryappender ALIAS ${opentelemetryappender})
if (UNICODE)
  target_compile_definitions (${opentelemetryappender} PUBLIC UNICODE)
  target_compile_definitions (${opentelemetryappender} PUBLIC _UNICODE)
  add_definitions (-UMBCS -U_MBCS)
endif (UNICODE)
if (BUILD_SHARED_LIBS)
  target_compile_definitions (${opentelemetryappender}
    PUBLIC LOG4CPLUS_OPENTELEMETRYAPPENDER_BUILD_DLL)
endif ()
target_compile_definitions (${opentelemetryappender}
  PRIVATE INSIDE_LOG4CPLUS_OPENTELEMETRYAPPENDER)
target_link_libraries (${opentelemetryappender}
  PUBLIC ${log4cplus}
  PUBLIC opentelemetry-cpp::api
  PUBLIC ${CMAKE_THREAD_LIBS_INIT})
if (LOG4CPLUS_WITH_OPENTELEMETRY_OTLP)
  target_link_libraries (${opentelemetryappender} PRIVATE opentelemetry-cpp::sdk)
  target_compile_definitions (${opentelemetryappender}
    PRIVATE LOG4CPLUS_WITH_OPENTELEMETRY_OTLP=1)
  if (LOG4CPLUS_WITH_OPENTELEMETRY_OTLP_HTTP)
    target_compile_definitions (${opentelemetryappender}
      PRIVATE LOG4CPLUS_WITH_OPENTELEMETRY_OTLP_HTTP=1)
    target_link_libraries (${opentelemetryappender}
      PRIVATE opentelemetry-cpp::otlp_http_log_record_exporter)
  endif ()
  if (LOG4CPLUS_WITH_OPENTELEMETRY_OTLP_GRPC)
    target_compile_definitions (${opentelemetryappender}
      PRIVATE LOG4CPLUS_WITH_OPENTELEMETRY_OTLP_GRPC=1)
    target_link_libraries (${opentelemetryappender}
      PRIVATE opentelemetry-cpp::otlp_grpc_log_record_exporter)
  endif ()
endif ()

if (ANDROID)
  # Android does not seem to have SO version support.
elseif (WIN32)
  set_target_properties (${opentelemetryappender} PROPERTIES
    VERSION "${log4cplus_version_major}.${log4cplus_version_minor}.${log4cplus_version_patch}")
else ()
  set_target_properties (${opentelemetryappender} PROPERTIES
    SOVERSION "${log4cplus_soversion}")
endif ()

if (APPLE)
  set_target_properties (${opentelemetryappender} PROPERTIES
    MACHO_CURRENT_VERSION "${log4cplus_macho_current_version}"
    MACHO_COMPATIBILITY_VERSION "${log4cplus_macho_compatibility_version}")
endif ()

if (WIN32)
  set_target_properties (${opentelemetryappender} PROPERTIES
    DEBUG_POSTFIX "D")
endif ()

install (TARGETS ${opentelemetryappender}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install (FILES
  ../include/log4cplus/opentelemetryappender.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/log4cplus)
