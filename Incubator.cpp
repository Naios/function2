
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
    namespace fn_types
    {
        struct member
        {
            void operator() () { }
        };

        using t_member = decltype(&member::operator());

        struct const_member
        {
            void operator() () const { }
        };

        using t_const_member = decltype(&const_member::operator());

        using td_const_member = std::decay_t<t_const_member>;

        

        struct static_member
        {
            static void my_fn() { }
        };

        using t_static_member = decltype(static_member::my_fn);

        void my_fn()
        {
        }

        using t_my_fn = decltype(my_fn);
    }

    template<typename T>
    void blub12345(T t)
    {
        return t();
    }

    void test1234()
    {


        auto up = std::make_unique<int>(0);

        auto i = make_function([]
        {
        });

        blub12345([]
        {
        });

        function<void(int, int)> fn;

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


void test_incubator()
{
    fn_types::td_const_member slkjalakjsalk = nullptr;

    static_assert(detail::unwrap_traits::unwrap_trait<fn_types::t_member>::is_member, "huhu");

    int i = 0;
}
