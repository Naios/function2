
//  Copyright 2015-2019 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#include "function2-test.hpp"

ALL_LEFT_TYPED_TEST_CASE(MultiSignatureTests)

TYPED_TEST(MultiSignatureTests, CanInvokeMultipleSignatures) {
  typename TestFixture::template left_multi_t<bool(std::true_type) const,
                                              bool(std::false_type) const>
      left = fu2::overload([](std::true_type) { return true; },
                           [](std::false_type) { return false; });

  EXPECT_TRUE(left(std::true_type{}));
  EXPECT_FALSE(left(std::false_type{}));
}

TYPED_TEST(MultiSignatureTests, CanInvokeGenericSignatures) {
  typename TestFixture::template left_multi_t<bool(std::true_type) const,
                                              bool(std::false_type) const>
      left = [](auto value) { return bool(value); };

  EXPECT_TRUE(left(std::true_type{}));
  EXPECT_FALSE(left(std::false_type{}));
}
