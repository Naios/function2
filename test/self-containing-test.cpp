
//  Copyright 2015-2019 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#include "function2/function2.hpp"

bool testSelfContaining() {
  fu2::function<bool()> first = [] { return true; };
  fu2::unique_function<bool()> second = first;
  return first() && second();
}
