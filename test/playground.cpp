
//  Copyright 2015-2017 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#include "function2/function2.hpp"

void here() {
}

int main(int, char**) {
  fu2::unique_function_view<void()> my_view = here;

  auto fn = my_view.acquire();

  return 0;
}
