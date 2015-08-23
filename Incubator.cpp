
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

#include <iostream>
#include <type_traits>
#include <functional>
#include <string>
#include <memory>

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

template<typename T>
using unwrap = ::my::detail::unwrap_traits::unwrap<T>;

struct virtual_check
{
    virtual ~virtual_check()
    {
        int i = 0;
    }

    virtual int operator() () = 0;
};

auto deleter = [](int* ptr)
{
    delete ptr;
};

struct virtual_check_impl
    : virtual_check
{
    int i;

    std::unique_ptr<int, decltype(deleter)> up;

    virtual_check_impl(int _i)
        : i(_i), up(new int(20), deleter) { }

    virtual ~virtual_check_impl()
    {
        i = 0;
    }

    int operator() () override
    {
        return i;
    }
};

template<std::size_t Capacity, typename Base, typename Pointer = Base*>
class optimized_allocator
{
    std::uint8_t _dig[Capacity];

    bool _on_heap;

    Pointer _ptr;

    template<typename T>
    struct is_local_allocateable
    {
        static constexpr bool value =
            sizeof(T) <= sizeof(Base)
            && (std::alignment_of<Base>::value % std::alignment_of<T>::value) == 0;
    };

    void weak_deallocate()
    {
        if (_ptr)
        {
            if (_on_heap)
                delete _ptr;
            else
                _ptr->~Base();
        }
    }

public:
    optimized_allocator()
        : _ptr(nullptr) { }

    template<typename T, typename... Args>
    optimized_allocator(Args&&... args)
        : _ptr(nullptr) { }

    ~optimized_allocator()
    {
        weak_deallocate();
    }

    /// Direct allocate (use capacity)
    template<typename T, typename... Args>
    auto allocate(Args&&... args)
        -> std::enable_if_t<is_local_allocateable<T>::value, void>
    {
        weak_deallocate();

        _ptr = static_cast<Pointer>(&_dig);

        new (_ptr) T(std::forward<Args>(args)...);
    }

    /// Heap allocate
    template<typename T, typename... Args>
    auto allocate(Args&&... args)
        -> std::enable_if_t<!is_local_allocateable<T>::value, void>
    {
        weak_deallocate();

        _ptr = new T(std::forward<Args>(args)...);
    }

    void deallocate()
    {
        weak_deallocate();
        _ptr = nullptr;
    }

    auto get()
    {
        return _ptr;
    }
};


void test_incubator()
{  
    fn_test_types::volatile_tests t1;

    t1.access_a();
    t1.access_b();
    
    t1.access_a_vol();
    t1.access_b_vol();

    fn_test_types::volatile_tests volatile t2;

    // t2.access_a();
    // t2.access_b();

    t2.access_a_vol();
    t2.access_b_vol();

    // Const lambda function
    auto lam1 = [] { };
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

    /*
    static_assert(unwrap<decltype(&function<int() const>::operator())>::is_member,
        "check failed!");
    static_assert(unwrap<decltype(&function<int() const>::operator())>::is_const,
        "check failed!");
    static_assert(!unwrap<decltype(&function<int() const>::operator())>::is_volatile,
        "check failed!");
        */

    
    function<void(int, float) const> fn0;
    // fn0(1, 1);

    unique_function<void(std::string const&) const> fn2;
    // fn2("hey");

    typedef decltype(&function<int()>::operator()) hey;

    auto lam3 = []
    {
    };

    auto fnbig = make_function<decltype(lam3)&&, 40>(std::move(lam3));

    // Static test: make_function call with mutable functional
    {
        auto fn = make_function([] () mutable
        {
        });

        // fn();

        static_assert(std::is_same<decltype(fn), function<void()>>::value, "check failed!");
    }

    // auto sf = make_function(&fn_test_types::my_fn);

    // Static test: make_function call with non copyable functional
    {
        auto up = std::make_unique<int>(0);

        auto lam = [up = std::move(up)]
        {
            return *up;
        };

        auto fn = make_function(std::move(lam));

        // fn();

        static_assert(!std::is_copy_assignable<decltype(lam)>::value, "precondition failed!");
        static_assert(!std::is_copy_constructible<decltype(lam)>::value, "precondition failed!");

        static_assert(std::is_same<decltype(fn), unique_function<int() const>>::value, "check failed!");
    }
    
    function<void()> fn_test([]() mutable
    {
    });

    fn_test_types::empty_struct es;

    // function<void()> fn_test2(es);

    // fn_test();

    // auto const ptr = fn_test_types::my_fn;

    // make_function(ptr);

    //make_function(fn_test_types::empty_struct());

    // typedef detail::unwrap_traits::unwrap<decltype(ptr)> blub;

    // int is_m = blub::is_member;



    // auto const iii = std::is_class<decltype(ptr)>::value;

    // auto m = make_function(ptr);

    auto ttt = []
    {

    };

    static_assert(detail::is_functor<decltype(ttt)>::value, "blub");

    static_assert(!detail::is_functor<fn_test_types::empty_struct>::value, "blub");

    static_assert(detail::is_function_pointer<decltype(&fn_test_types::my_fn)>::value, "blub");

    unique_function<int() const> uffp(fn_test_types::my_fn);
    unique_function<int() const> uffp2(&fn_test_types::my_fn);

    function<void() const> res_ttt = make_function(std::move(ttt));

    function<void(int, int) const volatile> ffff;

    // ffff(1, 1);

    // auto mm = make_function(0);

    // function<void() const>::

    optimized_allocator<1, virtual_check, virtual_check const*> alc;

    alc.allocate<virtual_check_impl>(120);

    // int res12345 = (*alc.get())();

    using moo = unique_function<void()>::return_type;

    // SFO
    {
        char dig[200];

        auto ptr = reinterpret_cast<virtual_check*>(&dig);

        new (ptr) virtual_check_impl(77);

        // auto fp = &ptr->ret;

        int res = (*ptr)();

        ptr->~virtual_check();

        // breakpoint
        int i = 0;
    }
}
