# This consumer DLL tears down before the production log4cplus DLL. Keeping
# their code in separate modules is essential to test process-wide detection.
add_library (windows_shutdown_fixture SHARED fixture.cxx)
target_link_libraries (windows_shutdown_fixture PRIVATE ${log4cplus})

# The driver dynamically loads the fixture; it must not import its symbols,
# otherwise the FreeLibrary regression would leave the DLL loaded.
add_executable (windows_shutdown_test main.cxx)
add_dependencies (windows_shutdown_test windows_shutdown_fixture)
if (MINGW)
  target_link_options (windows_shutdown_test PRIVATE -municode)
endif ()

foreach (_mode exit exit_pending exitprocess_pending
    dll_initializer dll_initializer_exitprocess
    dll_deinitialize dll_deinitialize_exitprocess
    normal explicit atexit unload dll_unload)
  foreach (_shrink 0 1)
    set (_test "windows_shutdown_${_mode}_${_shrink}")
    add_test (NAME ${_test} COMMAND windows_shutdown_test
      --parent $<TARGET_FILE:windows_shutdown_fixture> ${_mode} ${_shrink})
    set_tests_properties (${_test} PROPERTIES TIMEOUT 25)
  endforeach ()
endforeach ()
