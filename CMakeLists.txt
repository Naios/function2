project(Playground CXX)
cmake_minimum_required(VERSION 3.1)

include(CheckCXXCompilerFlag)
include(CheckCXXSourceRuns)
include(CheckIncludeFiles)

if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(PLATFORM 64)
else()
    set(PLATFORM 32)
endif()

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

  set(BOOST_DEBUG ON)

  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_RUNTIME OFF)

  if(DEFINED ENV{BOOST_ROOT})
    set(BOOST_ROOT $ENV{BOOST_ROOT})
    set(BOOST_LIBRARYDIR ${BOOST_ROOT}/lib${PLATFORM}-msvc-14.0)
  else()
    message(FATAL_ERROR "No BOOST_ROOT environment variable could be found! Please make sure it is set and the points to your Boost installation.")
  endif()

else()
  # Check C++14 Compiler support.
  CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)

  if(NOT COMPILER_SUPPORTS_CXX14)
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

set(Boost_ADDITIONAL_VERSIONS "1.59" "1.59.0")
find_package(Boost 1.57 REQUIRED system context)

include_directories(${Boost_INCLUDE_DIRS})

include_directories(testing)

file(GLOB TEST_SOURCES *.cpp *.hpp *.h)

add_executable(Playground ${TEST_SOURCES})

target_link_libraries(Playground
  ${Boost_LIBRARIES}
)
