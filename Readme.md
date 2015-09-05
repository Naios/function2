
# C++14 Function 2 - fu2::

[![Build Status](https://travis-ci.org/Naios/Function2.svg?branch=master)](https://travis-ci.org/Naios//Function2)

Provides two improved implementations of `std::function`:

- **copyable** `fu2::function`
- **move-only** `fu2::unique_function`

which are:

- **const** and **volatile** correct (qualifiers are part of the `operator()` signature).
- **compatible** to `std::function`.
- **adaptable** through `fu2::function_base` (internal capacity, copyable).
- **covered** by unit tests and continuous integration.
- **header only**, just copy and include `function.hpp` in your project.

## Table of Contents

* **[Documentation](#documentation)**
  * **[How to use](#how-to-use)**
  * **[Constructing a function](#constructing-a-function)**
  * **[Non copyable unique functions](#non-copyable-unique-functions)**
  * **[Converbility of functions](#converbility-of-functions)**
* **[Coverage and runtime checks](#coverage-and-runtime-checks)**
* **[Compatibility](#compatibility)**
* **[License](#licence)**

## Documentation

### How to use

Function2 is implemented in one header only file `function.hpp`, no compilation is required.
Just drop and include `function.hpp` to start!

### Constructing a function

`fu2::function` and `fu2::unique_function` (non copyable) are easy to use:

```c++
fu2::function<void() const> fun = []
{
	// ...
};

// fun provides void operator()() const now
fun();
```

### Non copyable unique functions

`fu2::unique_function` also works with non copyable functors/ lambdas.

```c++
fu2::unique_function<bool() const> fun = [ptr = std::make_unique<bool>(true)]
{
    return *ptr;
};

// unique functions are move only
fu2::unique_function<bool() const> otherfun = std::move(fun):

otherfun();
```

### Converbility of functions

`fu2::function`, `fu2::unique_function` and `std::function` are convertible to each other when:

- The return type and parameter type match.
- The functions are both volatile or not.
- The functions are const correct:
  - `noconst = const`
  - `const = const`
  - `noconst = noconst`
- The functions are copyable correct
  - `unique = copyable`
  - `copyable = copyable`


| Cobvertible from \ to | fu2::function | fu2::unique_function | std::function |
|-----------------------|---------------|----------------------|---------------|
| fu2::function         | Yes           | Yes                  | Yes           |
| fu2::unique_function  | No            | Yes                  | No            |
| std::function         | Yes           | Yes                  | Yes           |

```c++
fu2::function<void()> fun = []{};
// OK
std::function<void()> std_fun = fun;
// OK
fu2::unique_function<void()> un_fun = fun;

// Error (non copyable -> copyable)
fun = un_fun;
// Error (non copyable -> copyable)
fun = un_fun;

```

## Coverage and runtime checks

Function2 is checked with unit tests and was tested with valgrind for memory leaks:

```
===============================================================================
All tests passed (73 assertions in 7 test cases)

==15215== LEAK SUMMARY:
==15215==    definitely lost: 0 bytes in 0 blocks
==15215==    indirectly lost: 0 bytes in 0 blocks
==15215==      possibly lost: 0 bytes in 0 blocks
==15215==    still reachable: 72,704 bytes in 1 blocks
==15215==         suppressed: 0 bytes in 0 blocks
==15215==
==15215== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

## Compatibility

Tested with:

- Visual Studio 2015
- Clang 3.6+
- GCC 4.9.2+

Every compiler with full C++14 support should work (`constexpr` excluded for msvc).

## License
Function2 is licensed under the Boost 1.0 License.
