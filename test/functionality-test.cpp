
//  Copyright 2015-2019 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#include "function2-test.hpp"

namespace {
/// Increases the linked counter once for every destruction
class DeallocatorChecker {
  std::shared_ptr<std::reference_wrapper<std::size_t>> checker_;

public:
  DeallocatorChecker(std::size_t& checker) {
    checker_ = std::shared_ptr<std::reference_wrapper<std::size_t>>(
        new std::reference_wrapper<std::size_t>(checker),
        [=](std::reference_wrapper<std::size_t>* ptr) {
          ++ptr->get();
          std::default_delete<std::reference_wrapper<std::size_t>>{}(ptr);
        });
  }

  std::size_t operator()() const {
    return checker_->get();
  }
};

struct VolatileProvider {
  bool operator()() volatile {
    return true;
  }
};

struct ConstProvider {
  bool operator()() const {
    return true;
  }
};

struct ConstVolatileProvider {
  bool operator()() const volatile {
    return true;
  }
};

struct RValueProvider {
  bool operator()() && {
    return true;
  }
};
} // namespace

ALL_LEFT_TYPED_TEST_CASE(AllSingleMoveAssignConstructTests)

TYPED_TEST(AllSingleMoveAssignConstructTests, AreEmptyOnDefaultConstruct) {
  typename TestFixture::template left_t<bool()> left;
  EXPECT_FALSE(left);
  EXPECT_TRUE(left.empty());
  left = returnTrue;
  EXPECT_FALSE(left.empty());
  EXPECT_TRUE(left);
  EXPECT_TRUE(left());
}

TYPED_TEST(AllSingleMoveAssignConstructTests, AreNonEmptyOnFunctorConstruct) {
  typename TestFixture::template left_t<bool()> left(returnTrue);
  EXPECT_TRUE(left);
  EXPECT_FALSE(left.empty());
  EXPECT_TRUE(left());
}

TYPED_TEST(AllSingleMoveAssignConstructTests, AreEmptyOnNullptrConstruct) {
  typename TestFixture::template left_t<bool()> left(nullptr);
  EXPECT_FALSE(left);
  EXPECT_TRUE(left.empty());
}

TYPED_TEST(AllSingleMoveAssignConstructTests, AreEmptyAfterNullptrAssign) {
  typename TestFixture::template left_t<bool()> left(returnTrue);
  EXPECT_TRUE(left);
  EXPECT_FALSE(left.empty());
  EXPECT_TRUE(left());
  left = nullptr;
  EXPECT_FALSE(left);
  EXPECT_TRUE(left.empty());
}

TYPED_TEST(AllSingleMoveAssignConstructTests,
           AreFreeingResourcesOnDestruction) {

  // Pre test
  {
    std::size_t deallocates = 0UL;

    {
      DeallocatorChecker checker{deallocates};
      ASSERT_EQ(deallocates, 0UL);
    }
    ASSERT_EQ(deallocates, 1UL);
  }

  // Real test
  {
    std::size_t deallocates = 0UL;

    {
      typename TestFixture::template left_t<std::size_t() const> left(
          DeallocatorChecker{deallocates});
      EXPECT_EQ(deallocates, 0UL);
    }
    EXPECT_EQ(deallocates, 1UL);
  }
}

TYPED_TEST(AllSingleMoveAssignConstructTests, AreConstructibleFromFunctors) {
  bool result = true;
  typename TestFixture::template left_t<bool(bool)> left(
      [=](bool in) { return result && in; });
  EXPECT_TRUE(left);
  EXPECT_FALSE(left.empty());
  EXPECT_TRUE(left(true));
}

TYPED_TEST(AllSingleMoveAssignConstructTests, AreConstructibleFromBind) {
  typename TestFixture::template left_t<bool()> left(
      std::bind(std::logical_and<bool>{}, true, true));
  EXPECT_TRUE(left);
  EXPECT_FALSE(left.empty());
  EXPECT_TRUE(left());
}

/*
TYPED_TEST(AllSingleMoveAssignConstructTests, ProvideItsSignatureAsOperator) {
  EXPECT_TRUE(
      (std::is_same<typename TestFixture::template left_t<void()>::return_type,
                    void>::value));

  EXPECT_TRUE(
      (std::is_same<typename TestFixture::template left_t<float()>::return_type,
                    float>::value));

  EXPECT_TRUE((std::is_same<typename TestFixture::template left_t<void(
                                float, double, int)>::argument_type,
                            std::tuple<float, double, int>>::value));

  EXPECT_TRUE((std::is_same<typename TestFixture::template left_t<
                                std::tuple<int, float>()>::argument_type,
                            std::tuple<>>::value));
}
*/

TYPED_TEST(AllSingleMoveAssignConstructTests, AcceptsItsQualifier) {
  {
    typename TestFixture::template left_t<bool() volatile> left;
    left = VolatileProvider{};
    EXPECT_TRUE(left());
  }

  {
    typename TestFixture::template left_t<bool() const> left;
    left = ConstProvider{};
    EXPECT_TRUE(left());
  }

  {
    typename TestFixture::template left_t<bool() const volatile> left;
    left = ConstVolatileProvider{};
    EXPECT_TRUE(left());
  }

  {
    typename TestFixture::template left_t<bool() &&> left;
    left = RValueProvider{};
    EXPECT_TRUE(std::move(left)());
  }
}

struct MyTestClass {
  bool result = true;

  bool getResult() const {
    return result;
  }
};

TYPED_TEST(AllSingleMoveAssignConstructTests, AcceptClassMethodPointers) {
  /*
  broken
  typename TestFixture::template left_t<bool(MyTestClass*)> left
    = &MyTestClass::getResult;

  MyTestClass my_class;

  EXPECT_TRUE(left(&my_class));*/
}
