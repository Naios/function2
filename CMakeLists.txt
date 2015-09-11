project(Function2 CXX)
cmake_minimum_required(VERSION 2.8)

include(CheckCXXCompilerFlag)
include(CheckCXXSourceRuns)
include(CheckIncludeFiles)

if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(PLATFORM 64)
else()
    set(PLATFORM 32)
endif()

if(MSVC)
  if(${MSVC_VERSION} LESS 1800)
    message(FATAL_ERROR "You are using an unsupported version of Visual Studio "
                        "which doesn't support all required C++11 features. "
                        "(Visual Studio 2015 (Version >= 1800) is required!)")

  endif()

  if(PLATFORM EQUAL 64)
    add_definitions("-D_WIN64")
  else()
    # ...
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")

else()
  # Check C++11 Compiler support.
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)

  if(NOT COMPILER_SUPPORTS_CXX11)
    message(FATAL_ERROR "Your compiler has no C++11 capability!")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  endif()

  # Enable full warnings
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")
endif()

unset(has_cxx14_lambda_capture CACHE)
check_cxx_source_compiles("
  #include <memory>
  int main()
  {
    auto fun = [ptr = std::make_unique<int>(0)]
    {
        return *ptr;
    };
    return fun();
  }"
has_cxx14_lambda_capture)

if (has_cxx14_lambda_capture)
  add_definitions("-DHAS_CXX14_LAMBDA_CAPTURE")
  message(STATUS "Detected support for C++14 extended lambda captures.")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(testing)

file(GLOB TEST_SOURCES testing/*.cpp *.hpp)

add_executable(Test ${TEST_SOURCES})
