message (STATUS "Threads: ${CMAKE_THREAD_LIBS_INIT}")
set (loggingserver_sources loggingserver.cxx)

message (STATUS "Sources: ${loggingserver_sources}")

set (loggingserver loggingserver${log4cplus_postfix})
add_executable (${loggingserver} ${loggingserver_sources})
if (UNICODE)
  target_compile_definitions (${loggingserver} PUBLIC UNICODE)
  target_compile_definitions (${loggingserver} PUBLIC _UNICODE)
  add_definitions (-UMBCS -U_MBCS)
endif (UNICODE)
target_link_libraries (${loggingserver} ${log4cplus})

install(TARGETS ${loggingserver} DESTINATION ${CMAKE_INSTALL_BINDIR})
