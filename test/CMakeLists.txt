add_library(gtest STATIC
  "${CMAKE_CURRENT_LIST_DIR}/googletest/googletest/src/gtest-all.cc"
  "${CMAKE_CURRENT_LIST_DIR}/googletest/googletest/src/gtest_main.cc")

target_include_directories(gtest
  PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/googletest/googletest"
  PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/googletest/googletest/include")

target_compile_definitions(gtest
  PUBLIC
    -DGTEST_HAS_PTHREAD=0
    -DGTEST_LANG_CXX11=1)

target_compile_features(gtest
  PUBLIC
    cxx_alias_templates
    cxx_auto_type
    cxx_constexpr
    cxx_decltype
    cxx_final
    cxx_lambdas
    cxx_nullptr
    cxx_override)
    
file(GLOB TEST_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_executable(function2_tests ${TEST_SOURCES})

target_link_libraries(function2_tests
  PRIVATE
    function2
    gtest)

add_test(NAME function2-unit-tests COMMAND function2_tests)
