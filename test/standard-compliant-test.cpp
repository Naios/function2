
//  Copyright 2015-2016 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#include "function2-test.hpp"

ALL_LEFT_TYPED_TEST_CASE(StandardCompliantTest)

TYPED_TEST(StandardCompliantTest, IsSwappable)
{
  // The standard only requires that functions
  // with the same signature are swappable
  typename TestFixture::template left_t<bool()> left = returnTrue;
  typename TestFixture::template left_t<bool()> right = returnFalse;
  EXPECT_TRUE(left());
  EXPECT_FALSE(right());
  left.swap(right);
  EXPECT_TRUE(right());
  EXPECT_FALSE(left());
  right.swap(left);
  EXPECT_TRUE(left());
  EXPECT_FALSE(right());
}

TYPED_TEST(StandardCompliantTest, IsAssignable)
{
  struct FakeAllocator { };

  typename TestFixture::template left_t<bool()> left;
  EXPECT_FALSE(left);
  left.assign(returnFalse, FakeAllocator{});
  EXPECT_FALSE(left());
  left.assign(returnTrue, FakeAllocator{});
  EXPECT_TRUE(left());
}
