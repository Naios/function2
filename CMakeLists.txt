project(Playground CXX)
cmake_minimum_required(VERSION 3.1)

include(CheckCXXCompilerFlag)
include(CheckCXXSourceRuns)
include(CheckIncludeFiles)

if(MSVC)
  if(${MSVC_VERSION} LESS 1900)
    message(FATAL_ERROR "You are using an unsupported version of Visual Studio "
                        "which doesn't support all required C++14 features. "
                        "(Visual Studio 2015 (Version >= 1900) is required!)")

  endif()

  if(PLATFORM EQUAL 64)
    add_definitions("-D_WIN64")
  else()
    # ...
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")

else()
  # Check C++11 Compiler support.
  CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX11)

  if(NOT COMPILER_SUPPORTS_CXX11)
    # If not given try to enable C++1y
    CHECK_CXX_COMPILER_FLAG("-std=c++1y" COMPILER_SUPPORTS_CXX1Y)

    if(NOT COMPILER_SUPPORTS_CXX1Y)
      message(FATAL_ERROR "Your compiler has no C++14 capability!")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
    endif()
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  endif()

  # Enable full warnings
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")
endif()

find_package(Boost 1.59 REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

include_directories(testing)

file(GLOB TEST_SOURCES *.cpp *.hpp *.h)

add_executable(Playground ${TEST_SOURCES})

#target_link_libraries(Playground
#)
