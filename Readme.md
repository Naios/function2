
# C++11 Function 2 - fu2::

[![Build Status](https://travis-ci.org/Naios/function2.svg?branch=master)](https://travis-ci.org/Naios/function2) [![Build status](https://ci.appveyor.com/api/projects/status/1tl0vqpg8ndccats?svg=true)](https://ci.appveyor.com/project/Naios/function2)  ![](https://img.shields.io/badge/License-Boost-blue.svg)

Provides two improved implementations of `std::function`:

- **copyable** `fu2::function`
- **move-only** `fu2::unique_function`

which are:

- **const**, **volatile** and **reference** correct (qualifiers are part of the `operator()` signature).
- **convertible** to and from `std::function`.
- **adaptable** through `fu2::function_base` (internal capacity, copyable).
- **covered** by unit tests and continuous integration.
- **header only**, just copy and include `function.hpp` in your project.


## Table of Contents
* **[Documentation](#documentation)**
  * **[How to use](#how-to-use)**
  * **[Constructing a function](#constructing-a-function)**
  * **[Non copyable unique functions](#non-copyable-unique-functions)**
  * **[Converbility of functions](#converbility-of-functions)**
  * **[Adapt function2](#adapt-function2)**
* **[Performance and optimization](#performance-and-optimization)**
  * **[Small functor optimization](#small-functor-optimization)**
  * **[Compiler optimization](#compiler-optimization)**
  * **[std::function vs fu2::function](#stdfunction-vs-fu2function)**
* **[Coverage and runtime checks](#coverage-and-runtime-checks)**
* **[Compatibility](#compatibility)**
* **[License](#licence)**
* **[Similar implementations](#similar-implementations)**

## Documentation

### How to use

**function2** is implemented in one header (`function.hpp`), no compilation is required.
Just copy the `function.hpp` header in your project and include it to start.
It's recommended to import the library as git submodule using CMake:

```sh
# Shell:
git submodule add https://github.com/Naios/function2.git
```

```cmake
# CMake file:
add_subdirectory(function2)
# function2 provides an interface target which makes it's
# headers available to all projects using function2
target_link_libraries(my_project function2)
```

Use `fu2::function` as a wrapper for copyable function wrappers and `fu2::unique_function` for move only types.
The standard implementation `std::function` and `fu2::function` are convertible to each other, see [the chapter converbility of functions](#converbility-of-functions) for details.

A function wrapper is declared as followed:
```c++
fu2::function<void(int, float) const>
// Return type ~^   ^     ^     ^
// Arguments ~~~~~~~|~~~~~|     ^
// Qualifier ~~~~~~~~~~~~~~~~~~~|
```

* **Return type**: The return type of the function to wrap.
* **Arguments**: The argument types of the function to wrap.
  Any argument types are allowed.
* **Qualifiers**: There are several qualifiers allowed:
  - **no qualifier** provides `ReturnType operator() (Args...)`
    - Can be assigned from const and no const objects (*mutable lambdas* for example).
  - **const** provides `ReturnType operator() (Args...) const`
    - Requires that the assigned functor is const callable (won't work with *mutable lambdas*),
  - **volatile** provides `ReturnType operator() (Args...) volatile`
    - Can only be assigned from volatile qualified functors.
  - **const volatile** provides `ReturnType operator() (Args...) const volatile`
    - Same as const and volatile together.
  - Also there is support for **r-value functions** `ReturnType operator() (Args...) &&`
    - one-shot functions which are invalidated after the first call.

To build the function2 unit tests you need to pull the submodules (gtest) and build function2 as CMake standalone project:

```sh
git submodule init
git submodule update
mkdir build
cd build
cmake ..
make test
```

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
- The functions are copyable correct when:
  - `unique = unique`
  - `unique = copyable`
  - `copyable = copyable`
- The functions are reference correct when:
  - `lvalue = lvalue`
  - `lvalue = rvalue`
  - `rvalue = rvalue`

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

### Adapt function2

function2 is adaptable through `fu2::function_base` which allows you to set:

- **Signature:** defines the signature of the function.
- **Copyable:** defines if the function is copyable or not.
- **Capacity:** defines the internal capacity used for [sfo optimization](#small-functor-optimization).
- **Throwing** defines if empty function calls throw an `fu2::bad_function_call` exception, otherwise `std::abort` is called.

The following code defines a function with a variadic signature which is copyable and sfo optimization is disabled:

```c++
template<typename Signature>
using my_function = fu2::function_base<Signature, 0UL, true>;
```

The following code defines a non copyable function which just takes 1 argument, and has a huge capacity for internal sfo optimization.
Also it must be called as r-value.

```c++
template<typename Arg>
using my_consumer = fu2::function_base<void(Arg)&&, 100UL, false>;

// Example
my_consumer<int, float> consumer = [](int, float) { }
std::move(consumer)(44, 1.7363f);
```

## Performance and optimization

### Small functor optimization

function2 uses small functor optimization like the most common `std::function` implementations which means it allocates a small internal capacity to evade heap allocation for small functors.

Smart heap allocation moves the inplace allocated functor automatically to the heap to speed up moving between objects.

It's possible to disable small functor optimization through setting the internal capacity to 0.

### Compiler optimization

Functions are heavily optimized by compilers see below:

```c++
int main(int argc, char**)
{
    fu2::function<int()> fun([=]
    {
        return argc + 20;
    });
    return fun();
}
```

[Clang 3.4+ and GCC 4.8+ with -O3 compile the following short x86 asm code](https://goo.gl/S3YC98):

```asm
main: # @main
	lea	eax, [rdi + 20]
	ret
```

(`std::function` [compiles into ~70 instructions](https://goo.gl/GO4G4b)).

### std::function vs fu2::function

```
Benchmark: Construct and copy function wrapper
    std::function:         172535130ns
    fu2::(unique)function: 112452593ns       +53%

Benchmark: Move function wrapper around
    std::function:         313446044ns
    fu2::(unique)function: 110743394ns       +183%

Benchmark: Invoke function wrapper
    std::function:         38555121ns
    fu2::(unique)function: 28355481ns        +35%
```

## Coverage and runtime checks

Function2 is checked with unit tests and valgrind (for memory leaks), where the unit tests provide coverage for all possible template parameter assignments:

```
[----------] Global test environment tear-down
[==========] 419 tests from 105 test cases ran. (1349 ms total)
[  PASSED  ] 419 tests.
==20005==
==20005== HEAP SUMMARY:
==20005==     in use at exit: 72,704 bytes in 1 blocks
==20005==   total heap usage: 15,475 allocs, 15,474 frees, 1,807,616 bytes allocated
==20005==
==20005== LEAK SUMMARY:
==20005==    definitely lost: 0 bytes in 0 blocks
==20005==    indirectly lost: 0 bytes in 0 blocks
==20005==      possibly lost: 0 bytes in 0 blocks
==20005==    still reachable: 72,704 bytes in 1 blocks
==20005==         suppressed: 0 bytes in 0 blocks
==20005==
==20005== For counts of detected and suppressed errors, rerun with: -v
==20005== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)


```

## Compatibility

Tested with:

- Visual Studio 2015+ Update 2
- Clang 3.4+
- GCC 4.9+

Every compiler with modern C++11 support should work.
Function2 only depends on the standard library.

## License
Function2 is licensed under the very permissive Boost 1.0 License.

## Similar implementations

There are similar implementations of a function wrapper:

- [pmed/fixed_size_function](https://github.com/pmed/fixed_size_function)
- stdex::function - A multi-signature function implementation.
- multifunction - Example from [Boost.TypeErasure](http://www.boost.org/doc/html/boost_typeerasure/examples.html#boost_typeerasure.examples.multifunction), another multi-signature function.
- std::function - [Standard](http://en.cppreference.com/w/cpp/utility/functional/function).
- boost::function - The one from [Boost](http://www.boost.org/doc/libs/1_55_0/doc/html/function.html).
- func::function - From this [blog](http://probablydance.com/2013/01/13/a-faster-implementation-of-stdfunction/).
- generic::delegate - [Fast delegate in C++11](http://codereview.stackexchange.com/questions/14730/impossibly-fast-delegate-in-c11), also see [here](https://code.google.com/p/cpppractice/source/browse/trunk/).
- ssvu::FastFunc - Another Don Clugston's FastDelegate, as shown [here](https://groups.google.com/a/isocpp.org/forum/#!topic/std-discussion/QgvHF7YMi3o).
- [cxx_function::function](https://github.com/potswa/cxx_function) - By David Krauss

Also check out the amazing [**CxxFunctionBenchmark**](https://github.com/jamboree/CxxFunctionBenchmark) which compares several implementations.
