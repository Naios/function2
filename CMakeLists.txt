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
    cxx_final
    cxx_lambdas
    cxx_generic_lambdas
    cxx_inline_namespaces
    cxx_noexcept
    cxx_variadic_templates
    cxx_defaulted_functions
    cxx_defaulted_move_initializers
    cxx_nullptr)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
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
