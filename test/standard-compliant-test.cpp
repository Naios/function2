
//  Copyright 2015-2019 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#include "function2-test.hpp"

ALL_LEFT_TYPED_TEST_CASE(StandardCompliantTest)

TYPED_TEST(StandardCompliantTest, IsSwappableWithMemberMethod) {
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

TYPED_TEST(StandardCompliantTest, IsSwappableWithStdSwap) {
  // The standard only requires that functions
  // with the same signature are swappable
  typename TestFixture::template left_t<bool()> left = returnTrue;
  typename TestFixture::template left_t<bool()> right = returnFalse;
  EXPECT_TRUE(left());
  EXPECT_FALSE(right());
  std::swap(left, right);
  EXPECT_TRUE(right());
  EXPECT_FALSE(left());
  std::swap(left, right);
  EXPECT_TRUE(left());
  EXPECT_FALSE(right());
}

TYPED_TEST(StandardCompliantTest, IsSwappableWithSelf) {
  typename TestFixture::template left_t<bool()> left;
  left.swap(left);
  EXPECT_FALSE(left);
  left = returnTrue;
  left.swap(left);
  EXPECT_TRUE(left);
  EXPECT_TRUE(left());
}

TYPED_TEST(StandardCompliantTest, IsAssignableWithMemberMethod) {
  typename TestFixture::template left_t<bool()> left;
  EXPECT_FALSE(left);
  left.assign(returnFalse, std::allocator<int>{});
  EXPECT_FALSE(left());
  left.assign(returnTrue, std::allocator<int>{});
  EXPECT_TRUE(left());
}

TYPED_TEST(StandardCompliantTest, IsCompareableWithNullptrT) {
  typename TestFixture::template left_t<bool()> left;
  EXPECT_TRUE(left == nullptr);
  EXPECT_TRUE(nullptr == left);
  EXPECT_FALSE(left != nullptr);
  EXPECT_FALSE(nullptr != left);
  left = returnFalse;
  EXPECT_FALSE(left == nullptr);
  EXPECT_FALSE(nullptr == left);
  EXPECT_TRUE(left != nullptr);
  EXPECT_TRUE(nullptr != left);
}
