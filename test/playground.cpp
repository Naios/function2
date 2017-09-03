//  Copyright 2015-2016 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#include "function2/function2.hpp"
// using namespace fu2::detail;

#include <malloc.h>
#include <type_traits>

enum class command_code { invoke, move, copy };

template <typename T> struct identity : std::common_type<T> {};
template <typename T> identity<T> identityOf() { return {}; }
template <typename T> identity<T> identityOf(T /*type*/) { return {}; }

template <typename ReturnType, typename... Args>
using invoke_type = void (*)(void* context);
using command_table_type = void (*)(void* context);

template <typename T> class data {
  void* context;
  command_table_type command_table;
};

struct backend {
  // template <typename T>
  // data allocate() { return {}; }
};

struct frontend {};

int main(int, char**) {
  auto res = std::get<int>(std::tuple<float, double>{});

  int i = 0;
}
