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
    cxx_decltype
    cxx_final
    cxx_lambdas
    cxx_nullptr
    cxx_override)

add_executable(function2_tests
  ${CMAKE_CURRENT_LIST_DIR}/../include/function2/function2.hpp
  ${CMAKE_CURRENT_LIST_DIR}/assign-and-constructible-test.cpp
  ${CMAKE_CURRENT_LIST_DIR}/build-test.cpp
  ${CMAKE_CURRENT_LIST_DIR}/empty-function-call-test.cpp
  ${CMAKE_CURRENT_LIST_DIR}/function2-test.hpp
  ${CMAKE_CURRENT_LIST_DIR}/functionality-test.cpp
  ${CMAKE_CURRENT_LIST_DIR}/noexcept-test.cpp
  ${CMAKE_CURRENT_LIST_DIR}/self-containing-test.cpp
  ${CMAKE_CURRENT_LIST_DIR}/standard-compliant-test.cpp
  ${CMAKE_CURRENT_LIST_DIR}/type-test.cpp
  ${CMAKE_CURRENT_LIST_DIR}/multi-signature-test.cpp
  ${CMAKE_CURRENT_LIST_DIR}/regressions.cpp
  ${CMAKE_CURRENT_LIST_DIR}/view-test.cpp
  ${CMAKE_CURRENT_LIST_DIR}/overload-test.cpp)

target_link_libraries(function2_tests
  PRIVATE
    function2
    gtest)

add_test(NAME function2-unit-tests COMMAND function2_tests)

add_executable(function2_playground
  ${CMAKE_CURRENT_LIST_DIR}/../include/function2/function2.hpp
  ${CMAKE_CURRENT_LIST_DIR}/playground.cpp)

target_link_libraries(function2_playground
  PRIVATE
    function2)
