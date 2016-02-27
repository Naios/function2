include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest
  ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest/include
)

add_definitions(
  -DGTEST_HAS_PTHREAD=0
  -DGTEST_LANG_CXX11=1
)

add_library(gtest
  ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest/src/gtest-all.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest/src/gtest_main.cc
)

file(GLOB TEST_SOURCES *.cpp *.hpp)
add_executable(function2_tests ${TEST_SOURCES})

target_link_libraries(function2_tests gtest)

add_test(NAME function2-unit-tests COMMAND function2_tests)
