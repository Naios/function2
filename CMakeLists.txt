project(function2 CXX)
cmake_minimum_required(VERSION 2.8)

enable_testing()

option(TESTS_NO_EXCEPTIONS "Test without exceptions" OFF)
option(TESTS_NO_DEATH_TESTS "Test without death tests" OFF)

if (TESTS_NO_EXCEPTIONS)
  message(STATUS "Testing with exceptions disabled")
  add_definitions(-DTESTS_NO_EXCEPTIONS)
endif()

if (TESTS_NO_DEATH_TESTS)
  message(STATUS "Disabled death tests")
  add_definitions(-DTESTS_NO_DEATH_TESTS)
endif()

include(cmake/CMakeLists.txt)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(test)
