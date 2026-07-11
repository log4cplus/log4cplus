cmake_minimum_required(VERSION 3.20)

project(log4cplus_package_consumer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

find_package(log4cplus CONFIG REQUIRED)

add_library(log4cplus_consumer SHARED consumer.cxx)
target_link_libraries(log4cplus_consumer PRIVATE log4cplus::log4cplus)
