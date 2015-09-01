
//  Copyright 2015 Denis Blank <denis.blank at outlook dot com>
//   Distributed under the Boost Software License, Version 1.0
//      (See accompanying file LICENSE_1_0.txt or copy at
//           http://www.boost.org/LICENSE_1_0.txt)

#include <chrono>
#include <string>
#include <iostream>
#include <vector>

#include "function.hpp"

#include <functional>

template<typename F>
struct CopyFunctions
{
    static void invoke()
    {
        int c = 0;

        for (int i = 0; i < 1000000; ++i)
        {
            F fun = [&c]
            {
                ++c;
            };

            fun();
        }
    }
};

template<typename F>
struct MoveFunctions
{
    static void invoke()
    {
        std::size_t c = 0;

        std::vector<int> vec = { 1 };

        F right = [&c, vec = std::move(vec)]
        {
            c += vec.size();
        };

        for (int i = 0; i < 1000000; ++i)
        {
            F fun = std::move(right);
            fun();
            right = std::move(fun);
        }
    }
};

template<typename T>
auto measure()
{
    auto const start = std::chrono::high_resolution_clock::now();

    T::invoke();

    auto const end = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

template<typename StdImpl, typename OwnImpl>
void take_time(std::string const& name)
{
    auto const left = measure<StdImpl>();
    auto const right = measure<OwnImpl>();

    std::cout << "========\nBenchmark: " << name << std::endl;
    std::cout << "std::function:         " << left << "ns." << std::endl;
    std::cout << "fu2::(unique)function: " << right << "ns." << std::endl;

    std::cout << " -> " << right / left << "%" << std::endl << std::endl;
}

void runBenchmark()
{
    take_time<CopyFunctions<std::function<void()>>, CopyFunctions<fu2::function_base<void(), 64UL, true>>>("Construct test");

    take_time<MoveFunctions<std::function<void()>>, MoveFunctions<fu2::function_base<void(), 0UL, false>>>("Move test");
}
