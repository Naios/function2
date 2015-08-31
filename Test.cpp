
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

#include "function.hpp"
#include <functional>
#include <memory>

#define CATCH_CONFIG_RUNNER

#include "catch.hpp"

void test_mockup();
void test_incubator();
void runBenchmark();

// Functions without sfo optimization
template<typename Signature>
using function = fu2::function_base<Signature, 0UL, true>;

template<typename Signature>
using unique_function = fu2::function_base<Signature, 0UL, false>;

// Functions with sfo optimization
static constexpr std::size_t testing_sfo_capacity = 256UL;

template<typename Signature>
using sfo_function = fu2::function_base<Signature, testing_sfo_capacity, true>;

template<typename Signature>
using sfo_unique_function = fu2::function_base<Signature, testing_sfo_capacity, false>;

constexpr std::size_t sz1 = sizeof(std::function<bool(int, float, long)>);
constexpr std::size_t sz2 = sizeof(std::function<void()>);

int main(int argc, char** argv)
{
    runBenchmark();
    // test_mockup();
    // test_incubator();

    std::cout << "sizeof(std::function<bool(int, float, long)>) == " << sz1 << std::endl;
    std::cout << "sizeof(std::function<void()>) == " << sz2 << std::endl;

    int const result = Catch::Session().run(argc, argv);

    // Attach breakpoint here ,-)
    return result;
}

bool true_function()
{
    return true;
}

bool false_function()
{
    return false;
}

TEST_CASE("Functions are callable", "[function<>]")
{
    bool is_set = false;

    auto lam = [&](bool test)
    {
        is_set = test;
        return test;
    };

    REQUIRE_FALSE(is_set);

    SECTION("Simple call test with function<bool(bool) const>")
    {
        function<bool(bool) const> fun(lam);

        REQUIRE(fun(true));
        REQUIRE(is_set);

        REQUIRE_FALSE(fun(false));
        REQUIRE_FALSE(is_set);
    }

    SECTION("Simple call test with unique_function<bool(bool) const>")
    {
        unique_function<bool(bool) const> ufun(std::move(lam));

        REQUIRE(ufun(true));
        REQUIRE(is_set);

        REQUIRE_FALSE(ufun(false));
        REQUIRE_FALSE(is_set);
    }
}

TEST_CASE("Functions are copy and moveable", "[function<>]")
{
    SECTION("Move construct between unique_function<bool() const>")
    {
        unique_function<bool() const> right([]
        {
            return true;
        });

        unique_function<bool() const> left(std::move(right));

        REQUIRE(left());
    }

    SECTION("Move assign between unique_function<bool() const>")
    {
        unique_function<bool() const> left;

        unique_function<bool() const> right([]
        {
            return true;
        });

        left = std::move(right);

        REQUIRE(left());
    }

    SECTION("Move assign a lambda bool() const to unique_function<bool() const>")
    {
        unique_function<bool() const> left;

        left = [up = std::make_unique<bool>(true)]
        {
            return *up;
        };

        REQUIRE(left());
    }

    SECTION("Move construct between function<bool() const>")
    {
        function<bool() const> right([]
        {
            return true;
        });

        function<bool() const> left(std::move(right));

        REQUIRE(left());
    }

    SECTION("Move assign between function<bool() const>")
    {
        function<bool() const> left;

        function<bool() const> right([]
        {
            return true;
        });

        left = std::move(right);

        REQUIRE(left());
    }

    SECTION("Copy construct between function<int()>")
    {
        int counter = 0;
        function<int()> right([counter] () mutable
        {
            return counter++;
        });

        REQUIRE(right() == 0);
        REQUIRE(right() == 1);
        REQUIRE(right() == 2);

        function<int()> left(right);
        REQUIRE(left() == 3);
        REQUIRE(left() == 4);
        
        REQUIRE(right() == 3);
        REQUIRE(right() == 4);

        REQUIRE(left() == 5);
    }

    SECTION("Copy assign between function<int()>")
    {
        int counter = 0;
        function<int()> left;
        function<int()> right([counter]() mutable
        {
            return counter++;
        });

        REQUIRE(right() == 0);
        REQUIRE(right() == 1);
        REQUIRE(right() == 2);

        left = right;
        REQUIRE(left() == 3);
        REQUIRE(left() == 4);

        REQUIRE(right() == 3);
        REQUIRE(right() == 4);

        REQUIRE(left() == 5);
    }

    SECTION("Copy assign between function<int()> and unique_function<int()>")
    {
        int counter = 0;
        unique_function<int()> left;
        function<int()> right([counter]() mutable
        {
            return counter++;
        });

        REQUIRE(right() == 0);
        REQUIRE(right() == 1);
        REQUIRE(right() == 2);

        left = right;

        REQUIRE(left() == 3);
        REQUIRE(left() == 4);

        REQUIRE(right() == 3);
        REQUIRE(right() == 4);

        REQUIRE(left() == 5);
    }

    SECTION("Copy construct between function<int()> and unique_function<int()>")
    {
        int counter = 0;

        function<int()> right([counter]() mutable
        {
            return counter++;
        });

        REQUIRE(right() == 0);
        REQUIRE(right() == 1);
        REQUIRE(right() == 2);

        unique_function<int()> left(right);

        REQUIRE(left() == 3);
        REQUIRE(left() == 4);

        REQUIRE(right() == 3);
        REQUIRE(right() == 4);

        REQUIRE(left() == 5);
    }
}

TEST_CASE("Functions are convertible to and from functors", "[function<>]")
{
    SECTION("Copy construct fu2::function from std::function")
    {
        std::function<bool()> right([]
        {
            return true;
        });

        function<bool()> left = right;

        REQUIRE(left());
    }

    SECTION("Copy assign fu2::function from std::function")
    {
        std::function<bool()> right([]
        {
            return true;
        });

        function<bool()> left;

        left = right;

        REQUIRE(left());
    }

    SECTION("Copy construct std::function<bool()> from fu2::function<bool()>")
    {
        function<bool()> right([]() mutable
        {
            return true;
        });

        std::function<bool()> left(right);
    
        REQUIRE(left());
    }

    SECTION("Copy assign std::function<bool()> from fu2::function<bool()>")
    {
        function<bool()> right([]() mutable
        {
            return true;
        });

        std::function<bool()> left;

        left = right;

        REQUIRE(left());
    }

    SECTION("Copy construct std::function<bool()> from fu2::function<bool() const>")
    {
        function<bool() const> right([]
        {
            return true;
        });

        std::function<bool()> left(right);

        REQUIRE(left());
    }

    SECTION("Copy assign std::function<bool()> from fu2::function<bool() const>")
    {
        function<bool() const> right([]
        {
            return true;
        });

        std::function<bool()> left;

        left = right;

        REQUIRE(left());
    }

    SECTION("Move construct fu2::function from std::function")
    {
        std::function<bool()> right([]
        {
            return true;
        });

        function<bool()> left = std::move(right);

        REQUIRE(left());
    }

    SECTION("Move assign fu2::function from std::function")
    {
        std::function<bool()> right([]
        {
            return true;
        });

        function<bool()> left;

        left = std::move(right);

        REQUIRE(left());
    }

    SECTION("Move construct std::function<bool()> from fu2::function<bool()>")
    {
        function<bool()> right([]() mutable
        {
            return true;
        });

        std::function<bool()> left(std::move(right));

        REQUIRE(left());
    }

    SECTION("Move assign std::function<bool()> from fu2::function<bool()>")
    {
        function<bool()> right([]() mutable
        {
            return true;
        });

        std::function<bool()> left;

        left = std::move(right);

        REQUIRE(left());
    }

    SECTION("Move construct std::function<bool()> from fu2::function<bool() const>")
    {
        function<bool() const> right([]
        {
            return true;
        });

        std::function<bool()> left(std::move(right));

        REQUIRE(left());
    }

    SECTION("Move assign std::function<bool()> from fu2::function<bool() const>")
    {
        function<bool() const> right([]
        {
            return true;
        });

        std::function<bool()> left;

        left = std::move(right);

        REQUIRE(left());
    }
}

TEST_CASE("Functions are convertible from function pointers", "[function<>]")
{
    SECTION("Copy construct fu2::function from function pointers")
    {
        function<bool()> left = true_function;

        REQUIRE(left());

        left = false_function;

        REQUIRE_FALSE(left());
    }
}

TEST_CASE("unique_function's are convertible to non copyable functors and from copyable functors", "[unique_function<>]")
{
    SECTION("Move construct fu2::function from non copyable lambda")
    {
        auto right = [up = std::make_unique<bool>(true)]
        {
            return *up;
        };

        unique_function<bool() const> left = std::move(right);

        REQUIRE(left());
    }

    SECTION("Move assign fu2::function from non copyable lambda")
    {
        auto right = [up = std::make_unique<bool>(true)]
        {
            return *up;
        };

        unique_function<bool() const> left;

        left = std::move(right);

        REQUIRE(left());
    }

    SECTION("Move construct fu2::unique_function from std::function")
    {
        std::function<bool()> right([]
        {
            return true;
        });

        unique_function<bool()> left = std::move(right);

        REQUIRE(left());
    }

    SECTION("Move assign fu2::unique_function from std::function")
    {
        std::function<bool()> right([]
        {
            return true;
        });

        unique_function<bool()> left;

        left = std::move(right);

        REQUIRE(left());
    }

    SECTION("Evade copy of implementations when move constructing")
    {
        function<bool()> right([store = std::make_shared<bool>(true)]
        {
            return store.unique() && *store;
        });

        function<bool()> left(std::move(right));

        REQUIRE(left());
    }

    SECTION("Evade copy of implementations when move assigning")
    {
        function<bool()> left;

        function<bool()> right([store = std::make_shared<bool>(true)]
        {
            return store.unique() && *store;
        });

        left = std::move(right);

        REQUIRE(left());
    }
}

TEST_CASE("Functions with SFO optimization", "[function<>]")
{
    SECTION("Function SFO correctness and correct deallocation.")
    {
        bool deleted = false;

        {
            sfo_function<bool()> left;

            std::shared_ptr<int> ptr(new int(77), [&deleted](int* p)
            {
                deleted = true;
                delete p;
            });

            left = [ptr = std::move(ptr)]
            {
                return *ptr == 77;
            };

            REQUIRE(left());
        }

        REQUIRE(deleted);
    }

    SECTION("Function SFO copying")
    {
        sfo_function<bool()> left;

        sfo_function<bool()> right([]
        {
            return true;
        });

        left = right;

        REQUIRE(left());
    }

    SECTION("Function SFO copying from no sfo functions")
    {
        sfo_function<bool()> left;

        function<bool()> right([]
        {
            return true;
        });

        left = right;

        REQUIRE(left());
    }
}

struct volatile_functor
{
    bool operator() () volatile
    {
        return true;
    }
};

struct const_volatile_functor
{
    bool operator() () const volatile
    {
        return true;
    }
};

TEST_CASE("Functions with volatile qualifier", "[function<>]")
{
    SECTION("Function accepts volatile qualifier")
    {
        function<bool() volatile> left = volatile_functor{};

        REQUIRE(left());
    }

    SECTION("Function accepts const volatile qualifier")
    {
        function<bool() const volatile> left = const_volatile_functor{};

        REQUIRE(left());
    }
}
