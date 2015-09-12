
# C++11 Function 2 - fu2::

[![Build Status](https://travis-ci.org/Naios/Function2.svg?branch=master)](https://travis-ci.org/Naios/Function2)

Provides two improved implementations of `std::function`:

- **copyable** `fu2::function`
- **move-only** `fu2::unique_function`

which are:

- **const** and **volatile** correct (qualifiers are part of the `operator()` signature).
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

Function2 is implemented in one header only file `function.hpp`, no compilation is required.
Just drop and include `function.hpp` to start!

Use `fu2::function` as a wrapper for copyable function wrappers and `fu2::unique_function` for move only types.
In most cases is `std::function` and `fu2::function` compatible, see [the chapter converbility of functions](#converbility-of-functions) for details.

A function wrapper is declared as followed:
```c++
fu2::function<void(int, float) const>
// Return type ~^   ^     ^     ^
// Arguments ~~~~~~~|~~~~~|     ^
// Qualifier ~~~~~~~~~~~~~~~~~~~|
```

#### Return type
The return type of the function to wrap.

#### Arguments
The argument types of the function to wrap.
Any argument types are allowed.

#### Qualifiers

There are several qualifiers allowed:

- **no qualifier** provides `ReturnType operator() (Args...)`
  - Can be assigned from const and no const objects (*mutable lambdas* for example).
- **const** provides `ReturnType operator() (Args...) const`
  - Requires that the assigned functor is const callable (won't work with *mutable lambdas*),
- **volatile** provides `ReturnType operator() (Args...) volatile`
  - Can only be assigned from volatile qualified functors.
- **const volatile** provides `ReturnType operator() (Args...) const volatile`
  - Same as const and volatile together.

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

### Adapt function2

function2 is adaptable through `fu2::function_base` which allows you to set:

- **Signature:** defines the signature of the function.
- **Capacity:** defines the internal capacity used for [sfo optimization](#small-functor-optimization).
- **Copyable:** defines if the function is copyable or not.

The following code defines a function with a variadic signature which is copyable and sfo optimization is disabled:

```c++
template<typename Signature>
using my_function = fu2::function_base<Signature, 0UL, true>;
```

The following code defines a non copyable function which just takes 1 argument, and has a huge capacity for internal sfo optimization.

```c++
template<typename Arg>
using my_consumer = fu2::function_base<void(Arg), 100UL, false>;
```

## Performance and optimization

### Small functor optimization

Function 2 uses small functor optimization like the most common std::function implementations which means it allocates a small internal capacity to evade heap allocation for small functors.

Smart heap allocation moves the inplace allocated functor automatically to the heap to speed up moving between objects.

### Compiler optimization

Unique functions are heavily optimized by compilers see below:

```c++
int main(int argc, char**)
{
    fu2::unique_function<int()> fun([=]
    {
        return argc + 10;
    );
    return fun() + fun();
}
```

[Clang 3.4+ with -O3 compiles it into](https://goo.gl/F7saQy):

```asm
main: # @main
    lea	eax, [rdi + rdi + 20]
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

- Visual Studio 2015+
- Clang 3.4+
- GCC 4.8+

Every compiler with full C++11 capability should work.
Function2 only depends on the standard library.

## License
Function2 is licensed under the Boost 1.0 License.

## Similar implementations

- [potswa/cxx_function](https://github.com/potswa/cxx_function)
- [pmed/fixed_size_function](https://github.com/pmed/fixed_size_function)
- [http://probablydance.com/2013/01/13/a-faster-implementation-of-stdfunction/](http://probablydance.com/2013/01/13/a-faster-implementation-of-stdfunction)
