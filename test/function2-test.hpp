
//  Copyright 2015-2019 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#ifndef FU2_INCLUDED_FUNCTION2_TEST_HPP__
#define FU2_INCLUDED_FUNCTION2_TEST_HPP__

#include "function2/function2.hpp"
#include "gtest/gtest.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>

/// A function which always returns true
constexpr bool returnTrue() noexcept {
  return true;
}
/// A function which always returns false
constexpr bool returnFalse() noexcept {
  return false;
}

template <typename Fn, bool Copyable, std::size_t Capacity, bool Throwing,
          bool Owning, typename... Additional>
using short_def =
    fu2::function_base<Owning, Copyable, fu2::capacity_fixed<Capacity>,
                       Throwing, false, Fn, Additional...>;

/// NonCopyable functions with several SFO capacities
template <typename Fn, bool Throwing = true, bool Owning = true,
          typename... Additional>
using unique_no_sfo = short_def<Fn, false, 0, Throwing, Owning, Additional...>;
template <typename Fn, bool Throwing = true, bool Owning = true,
          typename... Additional>
using unique_256_sfo =
    short_def<Fn, false, 256, Throwing, Owning, Additional...>;
template <typename Fn, bool Throwing = true, bool Owning = true,
          typename... Additional>
using unique_512_sfo =
    short_def<Fn, false, 512, Throwing, Owning, Additional...>;
/// Copyable functions with several SFO capacities
template <typename Fn, bool Throwing = true, bool Owning = true,
          typename... Additional>
using copyable_no_sfo = short_def<Fn, true, 0, Throwing, Owning, Additional...>;
template <typename Fn, bool Throwing = true, bool Owning = true,
          typename... Additional>
using copyable_256_sfo =
    short_def<Fn, true, 256, Throwing, Owning, Additional...>;
template <typename Fn, bool Throwing = true, bool Owning = true,
          typename... Additional>
using copyable_512_sfo =
    short_def<Fn, true, 512, Throwing, Owning, Additional...>;
/// std::function
template <typename Fn, bool Throwing = true, bool Owning = true, typename...>
using std_function = std::function<Fn>;

/// Adds given types to the type list
template <typename... Rest>
struct MergeTypes : std::common_type<std::tuple<Rest...>> {};

template <typename... Tuple, typename... Rest>
struct MergeTypes<std::tuple<Tuple...>, Rest...>
    : MergeTypes<Rest..., Tuple...> {};

template <typename Tuple>
struct TupleToTypes;

template <typename... Args>
struct TupleToTypes<std::tuple<Args...>>
    : std::common_type<testing::Types<Args...>> {};

/// Provides the left type which is used in this test case
template <template <typename, bool, bool, typename...> class Left>
struct LeftType {
  /// Left function type which is provided by this test case.
  /// The left type isn't assignable to the right type!
  template <typename Fn, bool Throwing = true, typename... Additional>
  using left_t = Left<Fn, Throwing, true, Additional...>;

  template <typename Fn, typename... Additional>
  using left_multi_t = Left<Fn, false, true, Additional...>;

  template <typename Fn, typename... Additional>
  using left_view_t = Left<Fn, false, false, Additional...>;
};

/// Provides the left and right type which is used in this test case
template <template <typename, bool, bool, typename...> class Left,
          template <typename, bool, bool, typename...> class Right>
struct LeftRightType : LeftType<Left> {
  /// Right function type which is provided by this test case.
  /// The right type is assignable to the left type.
  template <typename Fn, bool Throwing = true, typename... Additional>
  using right_t = Right<Fn, Throwing, true, Additional...>;

  template <typename Fn, typename... Additional>
  using right_multi_t = Right<Fn, false, true, Additional...>;
};

/// Base class for typed function tests
template <typename Provider>
struct FunctionTesterBase : Provider, testing::Test {};

/// Declares a typed test with the given name and types.
#define DEFINE_FUNCTION_TEST_CASE(TEST_CASE_NAME, EXPANDED_TYPES)              \
  template <typename Provider>                                                 \
  using TEST_CASE_NAME = FunctionTesterBase<Provider>;                         \
  TYPED_TEST_CASE(TEST_CASE_NAME, typename TupleToTypes<EXPANDED_TYPES>::type);

/// Testing copyable types which are used in a test case
/// when a left type is required only
/// The functions are only capable of wrapping a copyable functor.
using CopyableLeftExpandedTypes =
    std::tuple<LeftType<copyable_no_sfo>, LeftType<copyable_256_sfo>,
               LeftType<copyable_512_sfo>>;

/// Declares a typed test case where all possibilities of copyable
/// functions are used as left parameter.
/// The functions are only capable of wrapping a copyable functor.
#define COPYABLE_LEFT_TYPED_TEST_CASE(TEST_CASE_NAME)                          \
  DEFINE_FUNCTION_TEST_CASE(TEST_CASE_NAME, CopyableLeftExpandedTypes)

/// Testing unique types which are used in a test case
/// when a left type is required only
/// The functions are capable of wrapping a unique functor.
using UniqueLeftExpandedTypes =
    std::tuple<LeftType<unique_no_sfo>, LeftType<unique_256_sfo>,
               LeftType<unique_512_sfo>>;

/// Declares a typed test case where all possibilities of copyable sfo
/// functions are used as left parameter.
/// The functions are capable of wrapping a unique functor.
#define UNIQUE_LEFT_TYPED_TEST_CASE(TEST_CASE_NAME)                            \
  DEFINE_FUNCTION_TEST_CASE(TEST_CASE_NAME, UniqueLeftExpandedTypes)

/// Testing all types which are used in a test case
/// when a left type is required only.
/// The functions are only capable of wrapping a copyable functor.
using AllLeftExpandedTypes = typename MergeTypes<CopyableLeftExpandedTypes,
                                                 UniqueLeftExpandedTypes>::type;

/// Declares a typed test case where all possibilities of
/// functions are used as left parameter.
/// The functions are only capable of wrapping a copyable functor.
#define ALL_LEFT_TYPED_TEST_CASE(TEST_CASE_NAME)                               \
  DEFINE_FUNCTION_TEST_CASE(TEST_CASE_NAME, AllLeftExpandedTypes)

/// Testing copyable types which are used in a test case
/// when a left and a right type is required.
/// The right and left types are always copy assignable to each other.
/// The types are only capable of wrapping a copyable functor.
using CopyableLeftRightExpandedTypes = std::tuple<
    // copyable_no_sfo = ?
    LeftRightType<copyable_no_sfo, copyable_no_sfo>,
    LeftRightType<copyable_no_sfo, copyable_256_sfo>,
    LeftRightType<copyable_no_sfo, copyable_512_sfo>,
    LeftRightType<copyable_no_sfo, std_function>,
    // copyable_256_sfo = ?
    LeftRightType<copyable_256_sfo, copyable_no_sfo>,
    LeftRightType<copyable_256_sfo, copyable_256_sfo>,
    LeftRightType<copyable_256_sfo, copyable_512_sfo>,
    LeftRightType<copyable_256_sfo, std_function>,
    // copyable_256_sfo = ?
    LeftRightType<copyable_512_sfo, copyable_no_sfo>,
    LeftRightType<copyable_512_sfo, copyable_256_sfo>,
    LeftRightType<copyable_512_sfo, copyable_512_sfo>,
    LeftRightType<copyable_512_sfo, std_function>,
    // std::function = ?
    LeftRightType<std_function, copyable_no_sfo>,
    LeftRightType<std_function, copyable_256_sfo>,
    LeftRightType<std_function, copyable_512_sfo>,
    LeftRightType<std_function, std_function>>;

/// Declares a typed test case where all possibilities of copyable sfo
/// functions are used as left and right parameter,
/// The right and left types are always copy assignable to each other.
/// The types are only capable of wrapping a copyable functor.
#define COPYABLE_LEFT_RIGHT_TYPED_TEST_CASE(TEST_CASE_NAME)                    \
  DEFINE_FUNCTION_TEST_CASE(TEST_CASE_NAME, CopyableLeftRightExpandedTypes)

/// Testing unique types which are used in a test case
/// when a left and a right type is required.
/// The right and left types are always move assignable to each other.
/// The types are capable of wrapping an unique functor.
using UniqueLeftRightExpandedTypes = std::tuple<
    // unique_no_sfo = ?
    LeftRightType<unique_no_sfo, unique_no_sfo>,
    LeftRightType<unique_no_sfo, unique_256_sfo>,
    LeftRightType<unique_no_sfo, unique_512_sfo>,
    // unique_256_sfo = ?
    LeftRightType<unique_256_sfo, unique_no_sfo>,
    LeftRightType<unique_256_sfo, unique_256_sfo>,
    LeftRightType<unique_256_sfo, unique_512_sfo>,
    // unique_512_sfo = ?
    LeftRightType<unique_512_sfo, unique_no_sfo>,
    LeftRightType<unique_512_sfo, unique_256_sfo>,
    LeftRightType<unique_512_sfo, unique_512_sfo>>;

/// Declares a typed test case where all possibilities of unique sfo
/// functions are used as left and right parameter,
/// where the right parameter is always move assignable to the left one.
/// The types are capable of wrapping an unique functor.
#define UNIQUE_LEFT_RIGHT_TYPED_TEST_CASE(TEST_CASE_NAME)                      \
  DEFINE_FUNCTION_TEST_CASE(TEST_CASE_NAME, UniqueLeftRightExpandedTypes)

/// Testing types which are used in a test case
/// when a left and a right type is required.
/// The right type is always move assignable to the left one,
/// but not the other way round.
/// The types are only capable of wrapping a copyable functor.
using AllLeftRightExpandedTypes =
    typename MergeTypes<UniqueLeftRightExpandedTypes,
                        CopyableLeftRightExpandedTypes,
                        // unique_no_sfo = ?
                        LeftRightType<unique_no_sfo, copyable_no_sfo>,
                        LeftRightType<unique_no_sfo, copyable_256_sfo>,
                        LeftRightType<unique_no_sfo, copyable_512_sfo>,
                        LeftRightType<unique_no_sfo, std_function>,
                        // unique_256_sfo = ?
                        LeftRightType<unique_256_sfo, copyable_no_sfo>,
                        LeftRightType<unique_256_sfo, copyable_256_sfo>,
                        LeftRightType<unique_256_sfo, copyable_512_sfo>,
                        LeftRightType<unique_256_sfo, std_function>,
                        // unique_512_sfo = ?
                        LeftRightType<unique_512_sfo, copyable_no_sfo>,
                        LeftRightType<unique_512_sfo, copyable_256_sfo>,
                        LeftRightType<unique_512_sfo, copyable_512_sfo>,
                        LeftRightType<unique_512_sfo, std_function>>::type;

/// Declares a typed test case where all possibilities of
/// functions are used as left and right parameter,
/// where the right parameter is always move assignable to the left one.
/// The types are only capable of wrapping a copyable functor.
#define ALL_LEFT_RIGHT_TYPED_TEST_CASE(TEST_CASE_NAME)                         \
  DEFINE_FUNCTION_TEST_CASE(TEST_CASE_NAME, AllLeftRightExpandedTypes)

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

#endif // FU2_INCLUDED_FUNCTION2_TEST_HPP__
