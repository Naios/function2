
//  Copyright 2015 Denis Blank <denis.blank at outlook dot com>
//   Distributed under the Boost Software License, Version 1.0
//      (See accompanying file LICENSE_1_0.txt or copy at
//           http://www.boost.org/LICENSE_1_0.txt)

#include "function.hpp"
#include <functional>
#include <memory>

#define CATCH_CONFIG_RUNNER

#include "catch.hpp"

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

constexpr std::size_t sz3 = sizeof(fu2::function<bool(int, float, long)>);
constexpr std::size_t sz4 = sizeof(fu2::unique_function<void()>);

constexpr std::size_t sz5 = sizeof(fu2::function_base<bool(int, float, long), 0UL, true>);
constexpr std::size_t sz6 = sizeof(fu2::function_base<void(), 0UL, false>);

constexpr std::size_t pd1 = std::alignment_of<std::function<bool(int, float, long)>>::value;
constexpr std::size_t pd2 = std::alignment_of<std::function<void()>>::value;

constexpr std::size_t pd3 = std::alignment_of<fu2::function<bool(int, float, long)>>::value;
constexpr std::size_t pd4 = std::alignment_of<fu2::unique_function<void()>>::value;

constexpr std::size_t pd5 = std::alignment_of<fu2::function_base<bool(int, float, long), 0UL, true>>::value;
constexpr std::size_t pd6 = std::alignment_of<fu2::function_base<void(), 0UL, false>>::value;

int main(int argc, char** argv)
{
    runBenchmark();

    std::cout << "\nsizeof(std::function<bool(int, float, long)>) == " << sz1 << std::endl;
    std::cout << "sizeof(std::function<void()>) == " << sz2 << std::endl;

    std::cout << "\nsizeof(fu2::function<bool(int, float, long)>) == " << sz3 << std::endl;
    std::cout << "sizeof(fu2::unique_function<void()>) == " << sz4 << std::endl;

    std::cout << "\nsizeof(fu2::function<bool(int, float, long)>) (no sfo) == " << sz5 << std::endl;
    std::cout << "sizeof(fu2::unique_function<void()>) (no sfo) == " << sz6 << std::endl << std::endl;

    int const result = Catch::Session().run(argc, argv);

    unique_function<void()&&> fff([]() mutable
    {

    });

    std::move(fff)();

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

    SECTION("Simple call test with function<bool(bool)&&>")
    {
        function<bool(bool)&&> fun(lam);

        REQUIRE(std::move(fun)(true));
        REQUIRE(is_set);
    }

    SECTION("Simple call test with unique_function<bool(bool)&&>")
    {
        unique_function<bool(bool)&&> ufun(std::move(lam));

        REQUIRE(std::move(ufun)(true));
        REQUIRE(is_set);
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

#ifdef HAS_CXX14_LAMBDA_CAPTURE

    SECTION("Move assign a lambda bool() const to unique_function<bool() const>")
    {
        unique_function<bool() const> left;

        left = [up = std::make_unique<bool>(true)]
        {
            return *up;
        };

        REQUIRE(left());
    }

#endif // #ifdef HAS_CXX14_LAMBDA_CAPTURE

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

struct bind_class
{
    bool through(bool ret) const
    {
        return ret;
    }
};

TEST_CASE("Functions are convertible from templated functors", "[function<>]")
{
    SECTION("Tests with std::bind")
    {
        bind_class bc;

        function<bool()> left = std::bind(&bind_class::through, bc, true);

        REQUIRE(left());

        left = std::bind(&bind_class::through, bc, false);

        REQUIRE_FALSE(left());
    }

#ifdef HAS_CXX14_LAMBDA_CAPTURE

    SECTION("Tests with auto lambdas")
    {
        function<bool(bool)> left = [](auto ret)
        {
            return ret;
        };

        REQUIRE(left(true));
        REQUIRE_FALSE(left(false));
    }

#endif // #ifdef HAS_CXX14_LAMBDA_CAPTURE
}

TEST_CASE("unique_function's are convertible to non copyable functors and from copyable functors", "[unique_function<>]")
{
#ifdef HAS_CXX14_LAMBDA_CAPTURE

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

#endif // #ifdef HAS_CXX14_LAMBDA_CAPTURE

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

#ifdef HAS_CXX14_LAMBDA_CAPTURE

    SECTION("Evade copy of implementations when move constructing")
    {
        function<bool()> right([store = std::make_shared<bool>(true)]
        {
            return store.unique() && *store;
        });

        function<bool()> left(std::move(right));

        REQUIRE(left());
    }

#endif // #ifdef HAS_CXX14_LAMBDA_CAPTURE

    SECTION("Evade copy of implementations when move assigning")
    {
        function<bool()> right;
        {
            auto store = std::make_shared<bool>(true);
            right = [=]
            {
                return store.unique() && *store;
            };
        }

        function<bool()> left(std::move(right));

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

            {
                std::shared_ptr<int> ptr(new int(77), [&deleted](int* p)
                {
                    deleted = true;
                    delete p;
                });

                left = [=]
                {
                    return *ptr == 77;
                };
            }

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

namespace fn_test_types
{
    struct member
    {
        int operator() ()
        {
            return 0;
        }
    };

    struct const_member
    {
        int operator() () const
        {
            return 0;
        }
    };

    struct volatile_member
    {
        int operator() () volatile
        {
            return 0;
        }
    };

    struct const_volatile_member
    {
        int operator() () const volatile
        {
            return 0;
        }
    };

    struct static_member
    {
        static int my_fn()
        {
            return 0;
        }
    };

    int my_fn()
    {
        return 0;
    }

    int my_fn_volatile()
    {
        return 0;
    }

    struct empty_struct
    {

    };

    struct volatile_tests
    {
        int a = 0;

        volatile int b = 0;

        int access_a()
        {
            return a;
        }

        int access_b()
        {
            return b;
        }

        int access_a_vol() volatile
        {
            return a;
        }

        int access_b_vol() volatile
        {
            return b;
        }
    };
}

/*
template<typename T>
using unwrap = fu2::detail::unwrap_traits::unwrap<T>;

TEST_CASE("Static asserts", "[function<>]")
{
    // Const lambda function
    auto lam1 = [] {};
    static_assert(unwrap<decltype(&decltype(lam1)::operator())>::is_member,
        "check failed!");
    static_assert(unwrap<decltype(&decltype(lam1)::operator())>::is_const,
        "check failed!");
    static_assert(!unwrap<decltype(&decltype(lam1)::operator())>::is_volatile,
        "check failed!");

    // Mutable lambda function
    auto lam2 = []() mutable -> int { return  0; };
    static_assert(unwrap<decltype(&decltype(lam2)::operator())>::is_member,
        "check failed!");
    static_assert(!unwrap<decltype(&decltype(lam2)::operator())>::is_const,
        "check failed!");
    static_assert(!unwrap<decltype(&decltype(lam2)::operator())>::is_volatile,
        "check failed!");

    // Class methods
    static_assert(unwrap<decltype(&fn_test_types::member::operator())>::is_member,
        "check failed!");
    static_assert(!unwrap<decltype(&fn_test_types::member::operator())>::is_const,
        "check failed!");
    static_assert(!unwrap<decltype(&fn_test_types::member::operator())>::is_volatile,
        "check failed!");

    // Class const methods
    static_assert(unwrap<decltype(&fn_test_types::const_member::operator())>::is_member,
        "check failed!");
    static_assert(unwrap<decltype(&fn_test_types::const_member::operator())>::is_const,
        "check failed!");
    static_assert(!unwrap<decltype(&fn_test_types::const_member::operator())>::is_volatile,
        "check failed!");

    // Class volatile methods
    static_assert(unwrap<decltype(&fn_test_types::volatile_member::operator())>::is_member,
        "check failed!");
    static_assert(!unwrap<decltype(&fn_test_types::volatile_member::operator())>::is_const,
        "check failed!");
    static_assert(unwrap<decltype(&fn_test_types::volatile_member::operator())>::is_volatile,
        "check failed!");

    // Class const volatile methods
    static_assert(unwrap<decltype(&fn_test_types::const_volatile_member::operator())>::is_member,
        "check failed!");
    static_assert(unwrap<decltype(&fn_test_types::const_volatile_member::operator())>::is_const,
        "check failed!");
    static_assert(unwrap<decltype(&fn_test_types::const_volatile_member::operator())>::is_volatile,
        "check failed!");

    // Static member functions
    static_assert(!unwrap<decltype(fn_test_types::static_member::my_fn)>::is_member,
        "check failed!");
    static_assert(!unwrap<decltype(fn_test_types::static_member::my_fn)>::is_const,
        "check failed!");
    static_assert(!unwrap<decltype(fn_test_types::static_member::my_fn)>::is_volatile,
        "check failed!");
}
*/