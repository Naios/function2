
//  Copyright 2015-2020 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#include "function2-test.hpp"

struct some_tag {};

ALL_LEFT_TYPED_TEST_CASE(AllEmptyFunctionCallTests)

TYPED_TEST(AllEmptyFunctionCallTests, CallSucceedsIfNonEmpty) {
  typename TestFixture::template left_t<bool()> left = returnTrue;
  EXPECT_TRUE(left());
}

TYPED_TEST(AllEmptyFunctionCallTests, CallSucceedsIfNonEmptyPtr) {
  using ptr_t = bool (*)();

  ptr_t ptr(returnTrue);

  typename TestFixture::template left_t<bool()> left = ptr;
  EXPECT_TRUE(left());
}

TYPED_TEST(AllEmptyFunctionCallTests, CallSucceedsIfNonEmptyRef) {
  using ref_t = bool (&)();

  ref_t ref(returnTrue);

  typename TestFixture::template left_t<bool()> left = ref;
  EXPECT_TRUE(left());
}

#if !defined(FU2_HAS_DISABLED_EXCEPTIONS)
#ifndef FU2_HAS_NO_FUNCTIONAL_HEADER
static_assert(std::is_same<fu2::bad_function_call, //
                           std::bad_function_call>::value,
              "Wrong fu2::bad_function_call exposed!");
#endif

TYPED_TEST(AllEmptyFunctionCallTests, CallThrowsIfEmpty) {
  typename TestFixture::template left_t<bool()> left;
  EXPECT_THROW(left(), fu2::bad_function_call);
}
#endif // FU2_HAS_DISABLED_EXCEPTIONS

#if !defined(FU2_HAS_NO_EMPTY_PROPAGATION)
COPYABLE_LEFT_TYPED_TEST_CASE(AllEmptyFunctionViewCallTests)

TYPED_TEST(AllEmptyFunctionViewCallTests, CallPropagatesEmpty) {
  typename TestFixture::template left_t<void(some_tag const&)> emptyf{};
  ASSERT_TRUE(emptyf.empty());

  typename TestFixture::template left_t<void(some_tag)> emptyf2 =
      std::move(emptyf);
  ASSERT_TRUE(emptyf2.empty());

  typename TestFixture::template left_view_t<void(some_tag const&)> emptyf3 =
      std::move(emptyf2);
  ASSERT_TRUE(emptyf3.empty());
}

TYPED_TEST(AllEmptyFunctionCallTests, CallPropagatesEmptyFnPtr) {
  using fn_t = void (*)(some_tag);
  fn_t const fn = nullptr;
  typename TestFixture::template left_t<void(some_tag)> emptyf(fn);
  ASSERT_TRUE(emptyf.empty());
}

struct my_callable {
  void operator()(some_tag const&) {
  }

  explicit operator bool() {
    return true;
  }
};

struct my_callable_empty {
  void operator()(some_tag const&) {
  }

  explicit operator bool() {
    return false;
  }
};

TYPED_TEST(AllEmptyFunctionCallTests, CallPropagatesEmptyCustom) {
  {
    typename TestFixture::template left_t<void(some_tag)> fn(my_callable{});
    ASSERT_FALSE(fn.empty());
  }

  {
    typename TestFixture::template left_t<void(some_tag)> fn(
        my_callable_empty{});
#if defined(FU2_HAS_LIMITED_EMPTY_PROPAGATION) ||                              \
    defined(FU2_HAS_NO_EMPTY_PROPAGATION)
    ASSERT_FALSE(fn.empty());
#else
    ASSERT_TRUE(fn.empty());
#endif
  }
}
#endif // FU2_HAS_NO_EMPTY_PROPAGATION
