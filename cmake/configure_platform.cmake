# Select the platform specific cmake file
if (WIN32)
  include("${CMAKE_SOURCE_DIR}/cmake/platform/windows.cmake")
elseif (UNIX)
  include("${CMAKE_SOURCE_DIR}/cmake/platform/unix.cmake")
else()
  message(FATAL_ERROR "Unknown platform!")
endif()
