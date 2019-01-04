
//  Copyright 2015-2019 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#include "function2-test.hpp"

COPYABLE_LEFT_TYPED_TEST_CASE(AllViewTests)

TYPED_TEST(AllViewTests, CallSucceedsIfNonEmpty) {
  typename TestFixture::template left_t<bool()> left = returnTrue;

  {
    typename TestFixture::template left_view_t<bool()> view(left);
    EXPECT_TRUE(view());
  }

  {
    typename TestFixture::template left_view_t<bool()> view;
    view = left;
    EXPECT_TRUE(view());
  }
}

TYPED_TEST(AllViewTests, CallSucceedsOfFunctionPointers) {
  typename TestFixture::template left_view_t<bool()> view(returnTrue);
  EXPECT_TRUE(view());
}

TYPED_TEST(AllViewTests, CallSucceedsIfCopyConstructed) {
  typename TestFixture::template left_t<bool()> left = returnTrue;
  typename TestFixture::template left_view_t<bool()> right(left);
  typename TestFixture::template left_view_t<bool()> view(left);
  EXPECT_TRUE(view());
}

TYPED_TEST(AllViewTests, CallSucceedsIfMoveConstructed) {
  typename TestFixture::template left_t<bool()> left = returnTrue;
  typename TestFixture::template left_view_t<bool()> right(left);
  typename TestFixture::template left_view_t<bool()> view(std::move(left));
  EXPECT_TRUE(view());
}

TYPED_TEST(AllViewTests, CallSucceedsIfCopyAssigned) {
  typename TestFixture::template left_t<bool()> left = returnTrue;
  typename TestFixture::template left_view_t<bool()> right(left);
  typename TestFixture::template left_view_t<bool()> view;
  view = right;
  EXPECT_TRUE(view());
}

TYPED_TEST(AllViewTests, CallSucceedsIfMoveAssigned) {
  typename TestFixture::template left_t<bool()> left = returnTrue;
  typename TestFixture::template left_view_t<bool()> right(left);
  typename TestFixture::template left_view_t<bool()> view;
  view = std::move(right);
  EXPECT_TRUE(view());
}

TYPED_TEST(AllViewTests, EmptyCorrect) {
  {
    typename TestFixture::template left_t<bool()> left = returnTrue;
    typename TestFixture::template left_view_t<bool()> view(left);
    EXPECT_TRUE(bool(view));
  }
  {
    typename TestFixture::template left_view_t<bool()> view;
    EXPECT_FALSE(bool(view));
  }
}

TYPED_TEST(AllViewTests, IsClearable) {
  typename TestFixture::template left_t<bool()> left = returnTrue;
  typename TestFixture::template left_view_t<bool()> view(left);
  EXPECT_TRUE(bool(view));
  EXPECT_TRUE(view());
  view = nullptr;
  EXPECT_FALSE(bool(view));
}

TYPED_TEST(AllViewTests, IsConstCorrect) {
  {
    typename TestFixture::template left_t<bool() const> left = returnTrue;
    typename TestFixture::template left_view_t<bool() const> view(left);
    EXPECT_TRUE(view());
  }

  {
    typename TestFixture::template left_t<bool() const> left = returnTrue;
    typename TestFixture::template left_view_t<bool()> view(left);
    EXPECT_TRUE(view());
  }

  {
    typename TestFixture::template left_view_t<bool() const> view(returnTrue);
    EXPECT_TRUE(view());
  }
}

TYPED_TEST(AllViewTests, IsVolatileCorrect) {
  {
    typename TestFixture::template left_t<bool() volatile> left = returnTrue;
    typename TestFixture::template left_view_t<bool() volatile> view(left);
    EXPECT_TRUE(view());
  }

  {
    typename TestFixture::template left_view_t<bool() volatile> view(
        returnTrue);
    EXPECT_TRUE(view());
  }
}

TYPED_TEST(AllViewTests, HasCorrectObjectSize) {
  typename TestFixture::template left_view_t<bool() volatile> view;
  EXPECT_EQ(sizeof(view), 2 * sizeof(void*));
}
