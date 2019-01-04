
//  Copyright 2015-2019 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#include "function2-test.hpp"

ALL_LEFT_TYPED_TEST_CASE(OverloadTests)

template <typename TestFixture>
struct FunctionProvider {
  bool OverloadedMethod(
      typename TestFixture::template left_t<void(std::false_type)>) {
    return false;
  }

  bool OverloadedMethod(
      typename TestFixture::template left_t<void(std::true_type)>) {
    return true;
  }
};

TYPED_TEST(OverloadTests, IsOverloadable) {
  // Test whether fu2::function supports overloading which isn't possible
  // with C++11 std::function implementations because of
  // a non SFINAE guarded templated constructor.
  FunctionProvider<TestFixture> provider;
  int i = 0;
  EXPECT_TRUE(provider.OverloadedMethod([&](std::true_type) { ++i; }));
}
