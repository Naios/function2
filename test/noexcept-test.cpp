
//  Copyright 2015-2019 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#include "function2-test.hpp"
#include <type_traits>

ALL_LEFT_TYPED_TEST_CASE(AllNoExceptTests)

TYPED_TEST(AllNoExceptTests, CallSucceedsIfNonEmpty) {
  typename TestFixture::template left_t<bool(), false> left = returnTrue;
  EXPECT_TRUE(left());
}

TEST(StrongNoExceptTests, GuaranteedNoExceptOperations) {
  using type = fu2::function_base<true, true, fu2::capacity_fixed<100U>, true,
                                  true, void()>;

  EXPECT_TRUE(std::is_nothrow_destructible<type>::value);
  EXPECT_TRUE(std::is_nothrow_move_assignable<type>::value);
  EXPECT_TRUE(std::is_nothrow_move_constructible<type>::value);
}

// Issue #5
// Death tests are causing issues when doing leak checks in valgrind
#ifndef TESTS_NO_DEATH_TESTS
TYPED_TEST(AllNoExceptTests, CallAbortsIfEmpty) {
  typename TestFixture::template left_t<bool(), false> left;
  EXPECT_DEATH(left(), "");
}
#endif // TESTS_NO_DEATH_TESTS

#ifdef FU2_HAS_CXX17_NOEXCEPT_FUNCTION_TYPE
TYPED_TEST(AllNoExceptTests, NoExceptCallSuceeds) {
  typename TestFixture::template left_t<int() noexcept> left = []() noexcept {
    return 12345;
  };
  ASSERT_EQ(left(), 12345);
}

#ifndef TESTS_NO_DEATH_TESTS
TYPED_TEST(AllNoExceptTests, CallAbortsIfEmptyAndNoExcept) {
  typename TestFixture::template left_t<bool() noexcept> left;
  EXPECT_DEATH(left(), "");
}
#endif // TESTS_NO_DEATH_TESTS
#endif // FU2_HAS_CXX17_NOEXCEPT_FUNCTION_TYPE
