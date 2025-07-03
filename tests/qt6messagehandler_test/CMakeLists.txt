log4cplus_add_test(qt6messagehandler_test main.cxx)
find_package(Qt6 REQUIRED COMPONENTS Core)
target_link_libraries (qt6messagehandler_test
  PUBLIC log4cplus::qt6debugappender
  PRIVATE Qt6::Core
  PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_include_directories (qt6messagehandler_test PUBLIC ${Qt6Core_INCLUDES})
target_compile_definitions (qt6messagehandler_test PUBLIC ${Qt6Core_DEFINITIONS})
