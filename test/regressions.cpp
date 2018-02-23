
//  Copyright 2015-2017 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#include <string>

#include "function2-test.hpp"

struct stateful_callable {
  std::string test;

  void operator()() {
  }
};

/// Iterator dereference (nullptr) crash in Visual Studio
///
/// This was caused through an issue with the allocated pointer swap on move
TEST(regression_tests, move_iterator_dereference_nullptr) {
  std::string test = "hey";
  fu2::function<void()> fn = stateful_callable{std::move(test)};

  auto fn2 = std::move(fn);
  (void)fn2;
}

int function_issue_7_regression(int& i) {
  return i;
}

/// The following code does not compile on
/// MSVC version 19.12.25830.2 (Visual Studio 2017 15.5.1):
///
/// https://github.com/Naios/function2/issues/7
TEST(regression_tests, reference_parameters_issue_7) {
  fu2::function<int(int&)> f = function_issue_7_regression;
  int i = 4384674;
  ASSERT_EQ(f(i), 4384674);
}
