
//  Copyright 2015-2020 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#include "function2-test.hpp"

bool testSelfContaining();

int scrn() {

  fu2::unique_function<void(int, int)> fun = [](int /*a*/, int /*b*/) {
    // ...
  };

  (void)fun;
  return 0;
}

// Issue #6
TEST(build_tests, the_header_is_self_containing) {
  EXPECT_TRUE(testSelfContaining());
}
