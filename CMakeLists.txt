cmake_minimum_required(VERSION 3.0)
project(function2 VERSION 2.2.0 LANGUAGES CXX)

add_library(function2 INTERFACE)
add_library(function2::function2 ALIAS function2)

target_include_directories(function2
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>)

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

# Create an install target
install(TARGETS function2
        EXPORT function2-config
        INCLUDES DESTINATION include)

install(EXPORT function2-config
        FILE function2-config.cmake
        NAMESPACE function2::
        DESTINATION share/function2/cmake)

install(DIRECTORY include/function2
        DESTINATION include FILES_MATCHING PATTERN "*.hpp")

install(FILES LICENSE.txt DESTINATION . RENAME function2-LICENSE.txt)
install(FILES Readme.md DESTINATION . RENAME function2-Readme.md)

# Setup CPack for bundling
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

# Since the header only library is platform independent
# we name the packages after the native line feed
if(WIN32)
  set(CPACK_SYSTEM_NAME "crlf")
else()
  set(CPACK_SYSTEM_NAME "lf")
endif()

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
