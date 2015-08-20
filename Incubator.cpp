
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

#include <boost/context/all.hpp>


#include <boost/function_types/components.hpp>
#include <boost/function.hpp>

#include <iostream>
#include <type_traits>
#include <functional>
#include <string>

#include "function.hpp"

using namespace my;

/*
 * - Const correct
 * - Move only
 * - Threadsafe
 */


/*
template<typename Fn>
inline auto impl_make_function(Fn&& functional)
    -> std::enable_if_t<
        !(std::is_copy_constructible<std::decay_t<Fn>>::value
        || std::is_copy_assignable<std::decay_t<Fn>>::value),
        decltype(auto)
       >
{
    return 1;
}

template<typename Fn>
inline auto impl_make_function(Fn&& functional)
-> std::enable_if_t<
    (std::is_copy_constructible<std::decay_t<Fn>>::value
    || std::is_copy_assignable<std::decay_t<Fn>>::value),
    decltype(auto)
   >
{
    return 1;
}
*/

namespace incubator
{
    template<typename T>
    void blub12345(T t)
    {
        return t();
    }

    void test1234()
    {


        auto up = std::make_unique<int>(0);

        blub12345([]
        {
        });

        function<void(int, int) const> fn;

        fn(1, 1);

        /*std::function<void()> fn2([up = std::move(up)]
        {

        })*/;

        // boost::function_types::components<decltype(&decltype(blub)::operator())>::type::
    }
    



} // namespace incubator

using namespace incubator;

boost::context::fcontext_t fc1 = nullptr, fcm = nullptr;

void f1(intptr_t)
{
    int i = 0;
    ++i;

    std::cout << "1" << std::endl;
    boost::context::jump_fcontext(&fc1, fcm, 0);
}

void test_context()
{
    std::size_t size(8192);
    void* sp1(std::malloc(size));
    fc1 = boost::context::make_fcontext(sp1, size, f1);

    boost::context::jump_fcontext(&fcm, fc1, 0);

    // std::free(sp1);

    int i = 0;
    ++i;
    std::cout << "2" << std::endl;
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

    int volatile my_fn_volatile()
    {
        return 0;
    }
}

template<typename T>
using unwrap = ::my::detail::unwrap_traits::unwrap<typename T>;

void test_incubator()
{  
    // Const lambda function
    auto lam1 = [] { };
    static_assert(unwrap<decltype(&decltype(lam1)::operator())>::is_member,
        "check failed!");
    static_assert(unwrap<decltype(&decltype(lam1)::operator())>::is_const,
        "check failed!");
    static_assert(!unwrap<decltype(&decltype(lam1)::operator())>::is_volatile,
        "check failed!");

    // Mutable lambda function
    auto lam2 = [] () mutable -> int {};
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

    /*
    static_assert(unwrap<decltype(&function<int() const>::operator())>::is_member,
        "check failed!");
    static_assert(unwrap<decltype(&function<int() const>::operator())>::is_const,
        "check failed!");
    static_assert(!unwrap<decltype(&function<int() const>::operator())>::is_volatile,
        "check failed!");
        */

    
    function<void(int, float) const> fn0;
    fn0(1, 1);

    non_copyable_function<void(std::string const&) const> fn2;
    fn2("hey");

    typedef decltype(&function<int()>::operator()) hey;



    // Static test: make_function call with non copyable functional 
    {
        auto up = std::make_unique<int>(0);

        auto fn = make_function([up = std::move(up)]
        {
        });

        static_assert(!std::is_copy_assignable<decltype(fn)>::value, "precondition failed!");
        static_assert(!std::is_copy_constructible<decltype(fn)>::value, "precondition failed!");

        static_assert(std::is_same<decltype(fn), non_copyable_function<void() const>>::value, "check failed!");
    }

    int i = 0;
}
