
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

#define CATCH_CONFIG_RUNNER

#include "catch.hpp"

void test_mockup();
void test_incubator();

using namespace fu2;

int main(int argc, char** argv)
{
    // test_mockup();

    // test_incubator();

    int const result = Catch::Session().run(argc, argv);

    // Attach breakpoint here ,-)
    return result;
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
    SECTION("Simple move test with unique_function<bool() const>")
    {
        unique_function<bool() const> left;

        unique_function<bool() const> right([]
        {
            return true;
        });

        left = std::move(right);

        REQUIRE(left());
    }

    SECTION("Simple copy test with function<bool() const>")
    {
        function<bool() const> left;

        function<bool() const> right([]
        {
            return true;
        });

        left = right;

        // REQUIRE(left());
    }
}
