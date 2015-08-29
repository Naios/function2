
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

#include <type_traits>

namespace mockup
{

}// namespace mockup

using namespace mockup;

struct IA
{
    virtual ~IA() { }

    virtual IA* dosomething() = 0;

    virtual void setsomething(IA* ptr) = 0;
};

struct IB : IA
{
    virtual ~IB() { }

    virtual IB* dosomething() = 0;

    virtual void setsomething(IB* ptr) = 0;
};

struct A : IA
{
    virtual ~A() { }

    IA* dosomething() override
    {
        return nullptr;
    }

    void setsomething(IA*) override
    {
        
    }
};

struct B : IB
{
    B() = default;

    virtual ~B() { }

    IB* dosomething() override
    {
        return nullptr;
    }

    void setsomething(IA* ptr) override
    {
        
    }

    void setsomething(IB* ptr) override
    {
        // new (static_cast<B*>(ptr)) B();
    }
};

struct empty
{
    virtual ~empty() { }

    virtual void do_s() = 0;
};

static constexpr auto esz = sizeof(empty);

static constexpr auto is_empty = std::is_empty<empty>::value;

static constexpr auto sz = sizeof(B);

static constexpr auto szptr = sizeof(&B::dosomething);

static constexpr auto szvd = sizeof(void*);



    template<bool>
    struct copyable { };

    template <>
    struct copyable<false>
    {
        // Disables copy construct & copy assign
        copyable() = default;
        copyable(copyable const&) = delete;
        copyable(copyable&&) = default;
        copyable& operator=(copyable const&) = delete;
        copyable& operator=(copyable&&) = default;
    };

    template<typename A, typename B, typename C>
    struct storage_t
    {
        // Selecting the type depending on A, B and C.
        // Not covered by this sample.
        using type = int;
    };

    template<typename A, typename B, typename C, bool Copyable>
    class myclass
        : public copyable<Copyable>
    {
        storage_t<A, B, C> _storage;

    public:
        myclass() = default;

        // Comment this out to disable the error...
        template<typename A, typename B, typename C>
        myclass(myclass<A, B, C, true> const&) { }

        template<typename A, typename B, typename C, bool Copyable>
        myclass(myclass<A, B, C, Copyable>&&) { }

        template<typename A, typename B, typename C>
        myclass& operator= (myclass<A, B, C, true> const&) { return *this; }

        template<typename A, typename B, typename C, bool Copyable>
        myclass& operator= (myclass<A, B, C, Copyable>&&) { return *this; }
        
        // ... comment end
    };

void test_mockup()
{
    /////
    // Testing the copyable class
    myclass<int, int, int, true> mc1;

    // Copy construct
    myclass<int, int, int, true> mc2(mc1);
    // Copy assign
    mc1 = mc2;

    /////
    // Testing the move only class
    myclass<int, int, int, false> mc3;

    // Move construct
    myclass<int, int, int, false> mc4(std::move(mc3));
    // Move assign
    mc3 = std::move(mc4);

    // mc3 = mc4;


    // Not working part:
    // Copy and move from myclass with other template params
    myclass<int, int, float, true> mc5;
    // Error here:
    mc1 = mc5;

    IA* a = new A();

    IB* b = new B();

    IA* aa = a->dosomething();

    IB* bb = b->dosomething();
    IA* bbb = b->dosomething();

    
}
