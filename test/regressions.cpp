
//  Copyright 2015-2017 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#include <string>

#include "function2-test.hpp"

/// Iterator dereference (nullptr) crash in Visual Studio
///
/// This was caused through the fact that std::allocator allocates
/// uninitialized storage but calls operator::delete on the given
/// object which caused a double destruction.
TEST(regression_tests, move_iterator_dereference_nullptr) {
  std::string test = "hey";
  fu2::function<void()> fn = [test = std::move(test)]{};

  auto fn2 = std::move(fn);
  (void)fn2;
}
