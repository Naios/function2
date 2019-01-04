
//  Copyright 2015-2019 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#include "function2-test.hpp"

namespace {
/// Coroutine which increases it's return value by every call
class UniqueIncreasingCoroutine {
  std::unique_ptr<std::size_t> state = make_unique<std::size_t>(0);

public:
  UniqueIncreasingCoroutine() {
  }

  std::size_t operator()() {
    return (*state)++;
  }
};

/// Coroutine which increases it's return value by every call
class CopyableIncreasingCoroutine {
  std::size_t state = 0UL;

public:
  CopyableIncreasingCoroutine() {
  }

  std::size_t operator()() {
    return state++;
  }
};

/// Functor which returns it's shared count
class SharedCountFunctor {
  std::shared_ptr<std::size_t> state = std::make_shared<std::size_t>(0);

public:
  std::size_t operator()() const {
    return state.use_count();
  }
};
} // namespace

ALL_LEFT_RIGHT_TYPED_TEST_CASE(AllMoveAssignConstructTests)

TYPED_TEST(AllMoveAssignConstructTests, AreMoveConstructible) {
  typename TestFixture::template right_t<bool()> right = returnTrue;
  typename TestFixture::template left_t<bool()> left(std::move(right));
  EXPECT_TRUE(left());
}

TYPED_TEST(AllMoveAssignConstructTests, AreMoveAssignable) {
  typename TestFixture::template left_t<bool()> left;
  typename TestFixture::template right_t<bool()> right = returnTrue;
  left = std::move(right);
  EXPECT_TRUE(left());
}

TYPED_TEST(AllMoveAssignConstructTests, TransferStatesOnConstruct) {
  typename TestFixture::template left_t<std::size_t()> left;
  typename TestFixture::template right_t<std::size_t()> right =
      CopyableIncreasingCoroutine();
  EXPECT_EQ(right(), 0UL);
  EXPECT_EQ(right(), 1UL);
  left = std::move(right);
  EXPECT_EQ(left(), 2UL);
  EXPECT_EQ(left(), 3UL);
  EXPECT_EQ(left(), 4UL);
}

TYPED_TEST(AllMoveAssignConstructTests, TransferStatesOnAssign) {
  typename TestFixture::template right_t<std::size_t()> right =
      CopyableIncreasingCoroutine();
  EXPECT_EQ(right(), 0UL);
  EXPECT_EQ(right(), 1UL);
  typename TestFixture::template left_t<std::size_t()> left(std::move(right));
  EXPECT_EQ(left(), 2UL);
  EXPECT_EQ(left(), 3UL);
  EXPECT_EQ(left(), 4UL);
}

TYPED_TEST(AllMoveAssignConstructTests, AreEmptyAfterMoveConstruct) {
  typename TestFixture::template right_t<bool()> right = returnTrue;
  EXPECT_TRUE(right);
  EXPECT_TRUE(right());
  typename TestFixture::template left_t<bool()> left(std::move(right));
  EXPECT_FALSE(right);
  EXPECT_TRUE(left);
  EXPECT_TRUE(left());
}

TYPED_TEST(AllMoveAssignConstructTests, AreEmptyAfterMoveAssign) {
  typename TestFixture::template left_t<bool()> left;
  typename TestFixture::template right_t<bool()> right;
  EXPECT_FALSE(left);
  EXPECT_FALSE(right);
  right = returnTrue;
  EXPECT_TRUE(right);
  EXPECT_TRUE(right());
  left = std::move(right);
  EXPECT_FALSE(right);
  EXPECT_TRUE(left);
  EXPECT_TRUE(left());
}

UNIQUE_LEFT_RIGHT_TYPED_TEST_CASE(UniqueMoveAssignConstructTests)

TYPED_TEST(UniqueMoveAssignConstructTests, TransferStateOnMoveConstruct) {
  {
    typename TestFixture::template right_t<std::size_t()> right =
        UniqueIncreasingCoroutine();
    typename TestFixture::template left_t<std::size_t()> left(std::move(right));
    EXPECT_EQ(left(), 0UL);
  }

  {
    typename TestFixture::template right_t<std::size_t()> right =
        UniqueIncreasingCoroutine();
    EXPECT_EQ(right(), 0UL);
    EXPECT_EQ(right(), 1UL);
    EXPECT_EQ(right(), 2UL);
    typename TestFixture::template left_t<std::size_t()> left(std::move(right));
    EXPECT_EQ(left(), 3UL);
    EXPECT_EQ(left(), 4UL);
  }
}

TYPED_TEST(UniqueMoveAssignConstructTests, TransferStateOnMoveAssign) {
  {
    typename TestFixture::template left_t<std::size_t()> left;
    typename TestFixture::template right_t<std::size_t()> right =
        UniqueIncreasingCoroutine();
    left = std::move(right);
    EXPECT_EQ(left(), 0UL);
  }

  {
    typename TestFixture::template left_t<std::size_t()> left;
    typename TestFixture::template right_t<std::size_t()> right =
        UniqueIncreasingCoroutine();
    EXPECT_EQ(right(), 0UL);
    EXPECT_EQ(right(), 1UL);
    EXPECT_EQ(right(), 2UL);
    left = std::move(right);
    EXPECT_EQ(left(), 3UL);
    EXPECT_EQ(left(), 4UL);
  }
}

COPYABLE_LEFT_RIGHT_TYPED_TEST_CASE(CopyableCopyAssignConstructTests)

TYPED_TEST(CopyableCopyAssignConstructTests, AreCopyConstructible) {
  typename TestFixture::template right_t<bool()> right = returnTrue;
  typename TestFixture::template left_t<bool()> left(right);
  EXPECT_TRUE(left());
  EXPECT_TRUE(left);
  EXPECT_TRUE(right());
  EXPECT_TRUE(right);
}

TYPED_TEST(CopyableCopyAssignConstructTests, AreCopyAssignable) {
  typename TestFixture::template left_t<bool()> left;
  typename TestFixture::template right_t<bool()> right = returnTrue;
  EXPECT_FALSE(left);
  left = right;
  EXPECT_TRUE(left());
  EXPECT_TRUE(left);
  EXPECT_TRUE(right());
  EXPECT_TRUE(right);
}

TYPED_TEST(CopyableCopyAssignConstructTests, CopyStateOnCopyConstruct) {
  {
    typename TestFixture::template right_t<std::size_t()> right =
        CopyableIncreasingCoroutine();
    typename TestFixture::template left_t<std::size_t()> left(right);
    EXPECT_EQ(left(), 0UL);
    EXPECT_EQ(right(), 0UL);
  }

  {
    typename TestFixture::template right_t<std::size_t()> right =
        CopyableIncreasingCoroutine();
    EXPECT_EQ(right(), 0UL);
    EXPECT_EQ(right(), 1UL);
    EXPECT_EQ(right(), 2UL);
    typename TestFixture::template left_t<std::size_t()> left(right);
    EXPECT_EQ(left(), 3UL);
    EXPECT_EQ(right(), 3UL);
    EXPECT_EQ(left(), 4UL);
    EXPECT_EQ(right(), 4UL);
  }
}

TYPED_TEST(CopyableCopyAssignConstructTests, CopyStateOnCopyAssign) {
  {
    typename TestFixture::template left_t<std::size_t()> left;
    typename TestFixture::template right_t<std::size_t()> right =
        CopyableIncreasingCoroutine();
    left = right;
    EXPECT_EQ(left(), 0UL);
    EXPECT_EQ(right(), 0UL);
  }

  {
    typename TestFixture::template left_t<std::size_t()> left;
    typename TestFixture::template right_t<std::size_t()> right =
        CopyableIncreasingCoroutine();
    EXPECT_EQ(right(), 0UL);
    EXPECT_EQ(right(), 1UL);
    EXPECT_EQ(right(), 2UL);
    left = right;
    EXPECT_EQ(left(), 3UL);
    EXPECT_EQ(right(), 3UL);
    EXPECT_EQ(left(), 4UL);
    EXPECT_EQ(right(), 4UL);
  }
}
