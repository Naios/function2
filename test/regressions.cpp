
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

TEST(regression_tests, size_32) {
  fu2::function<void() const> fn;

  int es = fn.er();
  int fs = sizeof(fn);

  EXPECT_EQ(sizeof(fu2::function<void() const>), 32UL);
}
