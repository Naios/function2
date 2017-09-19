cmake_minimum_required(VERSION 3.0)
project(function2 CXX)

set(function2_VERSION_MAJOR 2)

add_library(function2 INTERFACE)

target_include_directories(function2
  INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/include")

target_compile_features(function2
  INTERFACE
    cxx_alias_templates
    cxx_auto_type
    cxx_constexpr
    cxx_decltype
    cxx_decltype_auto
    cxx_final
    cxx_lambdas
    cxx_lambda_init_captures
    cxx_generic_lambdas
    cxx_variadic_templates
    cxx_defaulted_functions
    cxx_nullptr
    cxx_trailing_return_types
    cxx_return_type_deduction)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  if (MSVC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  endif()

  enable_testing()

  option(TESTS_NO_EXCEPTIONS  "Test without exceptions"  OFF)
  option(TESTS_NO_DEATH_TESTS "Test without death tests" OFF)

  if (TESTS_NO_EXCEPTIONS)
    message(STATUS "Testing with exceptions disabled")
    add_definitions(-DTESTS_NO_EXCEPTIONS)
  endif()

  if (TESTS_NO_DEATH_TESTS)
    message(STATUS "Testing without death tests")
    add_definitions(-DTESTS_NO_DEATH_TESTS)
  endif()

  include(cmake/CMakeLists.txt)

  add_subdirectory(test)
endif ()
