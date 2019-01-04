
//  Copyright 2015-2019 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#include "function2-test.hpp"

ALL_LEFT_TYPED_TEST_CASE(AllTypeCheckTests)

TYPED_TEST(AllTypeCheckTests, IsDeclareableWithSupportedTypes) {
  {
    typename TestFixture::template left_t<bool()> left = returnTrue;
    EXPECT_TRUE(left());
  }
  {
    typename TestFixture::template left_t<bool() const> left = returnTrue;
    EXPECT_TRUE(left());
  }
  {
    typename TestFixture::template left_t<bool() const> const left = returnTrue;
    EXPECT_TRUE(left());
  }
  {
    typename TestFixture::template left_t<bool() volatile> left = returnTrue;
    EXPECT_TRUE(left());
  }
  {
    typename TestFixture::template left_t<bool() volatile> volatile left =
        returnTrue;
    EXPECT_TRUE(left());
  }
  {
    typename TestFixture::template left_t<bool() const volatile> left =
        returnTrue;
    EXPECT_TRUE(left());
  }
  {
    typename TestFixture::template left_t<
        bool() const volatile> const volatile left = returnTrue;
    EXPECT_TRUE(left());
  }
  {
    typename TestFixture::template left_t<bool()&> left = returnTrue;
    EXPECT_TRUE(left());
  }
  {
    typename TestFixture::template left_t<bool()&> left = returnTrue;
    EXPECT_TRUE(left());
  }
  {
    typename TestFixture::template left_t<bool() const&> left = returnTrue;
    EXPECT_TRUE(left());
  }
  {
    typename TestFixture::template left_t<bool() const&> const left =
        returnTrue;
    EXPECT_TRUE(left());
  }
  {
    typename TestFixture::template left_t<bool() volatile&> left = returnTrue;
    EXPECT_TRUE(left());
  }
  {
    typename TestFixture::template left_t<bool() volatile&> volatile left =
        returnTrue;
    EXPECT_TRUE(left());
  }
  {
    typename TestFixture::template left_t<bool() const volatile&> left =
        returnTrue;
    EXPECT_TRUE(left());
  }
  {
    typename TestFixture::template left_t<
        bool() const volatile&> const volatile left = returnTrue;
    EXPECT_TRUE(left());
  }
  {
    typename TestFixture::template left_t<bool()&&> left = returnTrue;
    EXPECT_TRUE(std::move(left)());
  }
  {
    typename TestFixture::template left_t<bool() const&&> left = returnTrue;
    EXPECT_TRUE(std::move(left)());
  }
  {
    typename TestFixture::template left_t<bool() const&&> const left =
        returnTrue;
    EXPECT_TRUE(std::move(left)());
  }
  {
    typename TestFixture::template left_t<bool() volatile&&> left = returnTrue;
    EXPECT_TRUE(std::move(left)());
  }
  {
    typename TestFixture::template left_t<bool() volatile&&> volatile left =
        returnTrue;
    EXPECT_TRUE(std::move(left)());
  }
  {
    typename TestFixture::template left_t<bool() const volatile&&> left =
        returnTrue;
    EXPECT_TRUE(std::move(left)());
  }
  {
    typename TestFixture::template left_t<
        bool() const volatile&&> const volatile left = returnTrue;
    EXPECT_TRUE(std::move(left)());
  }
}
