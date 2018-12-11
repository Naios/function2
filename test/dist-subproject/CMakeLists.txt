cmake_minimum_required(VERSION 3.2)
project(function2containtest VERSION 2.2.0 LANGUAGES CXX)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../..)

add_executable(function2-dist-subproject main.cpp)

target_link_libraries(function2-dist-subproject function2::function2)
