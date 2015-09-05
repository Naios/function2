
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

static constexpr std::size_t runs = 1000000;

template<typename F>
struct CopyFunctions
{
    static void invoke()
    {
        int c = 0;

        for (std::size_t i = 0; i < runs; ++i)
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

        for (std::size_t i = 0; i < runs; ++i)
        {
            F fun = std::move(right);
            fun();
            right = std::move(fun);
        }
    }
};

template<typename F>
struct InvokeFunctions
{
    static void invoke()
    {
        std::size_t c = 0;

        F right = [&c](int i)
        {
            c += i;
        };

        for (std::size_t i = 0; i < runs; ++i)
            right(i);
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

    auto pct = static_cast<int>((static_cast<double>(left) / static_cast<double>(right)) * 100) - 100;

    std::cout << "========\nBenchmark: " << name << std::endl << std::endl;
    std::cout << "std::function:         " << left << "ns" << std::endl;
    std::cout << "fu2::(unique)function: " << right << "ns\t " << (pct > 0 ? "+" : "") << pct << "%\n\n";
}

void runBenchmark()
{
    take_time<
        CopyFunctions<std::function<void()>>, CopyFunctions<fu2::function_base<void(), 64UL, true>>>
        ("Construct and copy function wrapper");

    take_time<
        MoveFunctions<std::function<void()>>, MoveFunctions<fu2::function_base<void(), 0UL, false>>>
        ("Move function wrapper around");

    take_time<
        InvokeFunctions<std::function<void(int)>>, InvokeFunctions<fu2::function_base<void(int), 64UL, true>>>
        ("Invoke function wrapper");

    std::cout << "========" << std::endl;
}
