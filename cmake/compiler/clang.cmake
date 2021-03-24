# Enable full warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wdocumentation -pedantic -Wextra")

if (FU2_WITH_NO_EXCEPTIONS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
  message(STATUS "Clang: Disabled exceptions")
endif()

