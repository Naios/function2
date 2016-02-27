
//  Copyright 2015-2016 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#include "function2-test.hpp"

ALL_LEFT_TYPED_TEST_CASE(AllNoExceptTests)

TYPED_TEST(AllNoExceptTests, CallSucceedsIfNonEmpty)
{
  typename TestFixture::template left_t<bool(), false> left = returnTrue;
  EXPECT_TRUE(left());
}

// Issue #5
// Death tests are causing issues when doing leak checks in valgrind
#ifndef TESTS_NO_DEATH_TESTS

TYPED_TEST(AllNoExceptTests, CallAbortsIfEmpty)
{
  typename TestFixture::template left_t<bool(), false> left;
  EXPECT_DEATH(left(), "");
}

#endif
