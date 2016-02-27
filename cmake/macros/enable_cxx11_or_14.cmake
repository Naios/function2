include(CheckCXXCompilerFlag)

# Enable C++11 or C++14 depending on the compilers capability
macro(enable_cxx11_or_14)
  # Check for C++14 support
  CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)

  if(NOT COMPILER_SUPPORTS_CXX14)
    # Check for C++11 support
    CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)

    if(COMPILER_SUPPORTS_CXX11)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    else()
      message(FATAL_ERROR "Your compiler has no C++11 capability!")
    endif()
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  endif()
endmacro()
