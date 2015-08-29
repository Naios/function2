
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

void test_mockup()
{
    IA* a = new A();

    IB* b = new B();

    IA* aa = a->dosomething();

    IB* bb = b->dosomething();
    IA* bbb = b->dosomething();
}
