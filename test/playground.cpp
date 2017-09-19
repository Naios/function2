//  Copyright 2015-2016 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#include "function2/function2.hpp"
#include <functional>

using namespace fu2::detail;

struct RValueProvider {
  bool operator()() const volatile {
    return true;
  }
};

template <typename T, typename = void>
struct tryit;

template <typename T>
struct tryit<
    T, std::enable_if_t<accepts_all<std::decay_t<T>, identity<bool()>>::value>>
    : std::true_type {};

struct My {
  bool method() {
    return true;
  }
};

int main(int, char**) {
  {
    fu2::unique_function<bool() const volatile> f;
    f.assign(RValueProvider{});

    fu2::unique_function<bool() const volatile> f2 = std::move(f);
    fu2::unique_function<bool() const volatile> f3(std::move(f2));

    // f3();
  }

  {
    fu2::function<bool()> f2;

    using trait = type_erasure::invocation_table::function_trait<bool()>;

    using callable = trait::callable<std::function<bool()>>;
    using args = trait::arguments;

    std::common_type<callable> ct;
    std::common_type<args> at;

    std::true_type tt4 = invocation::can_invoke<callable, args>{};

    f2.assign([] { return true; });

    f2.assign(std::function<bool()>{});
  }

  {
    My my;

    invocation::invoke(&My::method, &my);

    std::true_type tt = invocation::can_invoke<decltype(&My::method), identity<My*>>{};

    fu2::function<bool(My*)> fn;
    fn.assign(&My::method);
    fn(&my);
  }

  return 0;
}
