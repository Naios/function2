
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
    test_mockup();

    test_incubator();

    int const result = Catch::Session().run(argc, argv);

    // Attach breakpoint here ,-)
    return result;
}

TEST_CASE("Functions are callable", "[unique_function]")
{
    SECTION("Simple call test")
    {
        bool is_set = false;

        /*unique_function<void(bool) const> fun([&](bool test)
        {
            is_set = test;
        });

        fun(true);
        REQUIRE(is_set);

        fun(false);
        REQUIRE_FALSE(is_set);

        fun(true);
        REQUIRE(is_set);*/
    }
}
