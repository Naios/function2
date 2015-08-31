
/**
 * Copyright 2015 Denis Blank <denis.blank@outlook.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <chrono>
#include <string>
#include <iostream>

#include "function.hpp"

#include <functional>

template<typename F>
struct CopyFunctions
{
    static void invoke()
    {
        int c = 0;

        for (int i = 0; i < 100000; ++i)
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
        int c = 0;

        for (int i = 0; i < 100000; ++i)
        {
            F right = [&c]
            {
                ++c;
            };

            F fun = std::move(right);

            fun();
        }
    }
};

template<typename T>
auto measure()
{
    auto const start = std::chrono::high_resolution_clock::now();

    typename T::invoke();

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
    std::cout << "fu2::(unique)function: " << right << "ns." << std::endl  << std::endl;
}

void runBenchmark()
{
    take_time<CopyFunctions<std::function<void()>>, CopyFunctions<fu2::function_base<void(), 100, true>>>("Construct test");

    take_time<CopyFunctions<std::function<void()>>, CopyFunctions<fu2::unique_function<void()>>>("Move test");
}
