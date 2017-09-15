//  Copyright 2015-2016 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#include "function2/function2.hpp"

using namespace fu2::detail;

struct RValueProvider {
  bool operator()() const volatile&& {
    return true;
  }
};

/*
struct fn1 {
  template <typename T,
            std::enable_if_t<type_erasure::invocation_table::function_trait<
                void()>::is_accepting<std::decay_t<T>>::value>* = nullptr>
  void assign(T&& callable) {
  }
};

template <typename T, typename... Args>
struct can_accept_all_impl;
template <typename T, typename First, typename... Args>
struct can_accept_all_impl<T, First, Args...>
    : std::conditional_t<First::template is_accepting<T>::value,
                         can_accept_all_impl<T, Args...>, std::false_type> {};
template <typename T>
struct can_accept_all_impl<T> : std::true_type {};

template <typename T, typename... Args>
using can_accept_all = can_accept_all_impl<
    T, type_erasure::invocation_table::function_trait<Args>...>;

struct fn2 {
  template <typename T,
            std::enable_if_t<can_accept_all<std::decay_t<T>, void()>::value>* =
                nullptr>
  void assign(T&& callable) {
  }
};
*/

template <typename T, typename Signature,
          typename trait =
              type_erasure::invocation_table::function_trait<Signature>>
struct accepts_one
    : invocation::can_invoke<typename trait::template callable<T>,
                             typename trait::arguments> {};

template <typename T, typename Args, typename = void>
struct accepts_all : std::false_type {};
template <typename T, typename... Args>
struct accepts_all<
    T, identity<Args...>,
    always_void_t<std::enable_if_t<accepts_one<T, Args>::value>...>>
    : std::true_type {};

int main(int, char**) {

  using trait =
      type_erasure::invocation_table::function_trait<bool() const volatile&&>;

  using callable = trait::callable<RValueProvider>;
  using args = trait::arguments;

  std::true_type tt = invocation::can_invoke<callable, args>{};

  std::true_type tt2 =
      accepts_all<RValueProvider, identity<bool() const volatile&&>>{};

  /*fu2::unique_function<bool() const volatile&&> f;
  f.assign(RValueProvider{});*/

  /*std::true_type t = type_erasure::invocation_table::function_trait<
      bool() &&>::is_accepting<std::decay_t<RValueProvider>>{};*/

  /*RValueProvider pr;

  auto res = invocation::invoke(static_cast<RValueProvider&&>(pr));*/

  // fu2::unique_function<bool()> f2;
  // f2.assign([] { return true; });

  /*auto cal = []() {

  };

  fn2 f;
  f.assign(cal);*/

  return 0;
}
