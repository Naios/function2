
# C++14 Function 2 - fu2::

Provides two improved implementations of `std::function`:

- **copyable** `fu2::function`
- **move-only** `fu2::unique_function`

which are:

- **const** and **volatile** correct (qualifiers are part of the signature).
- **compatible** to `std::function`.
- **adaptable** through `fu2::function_base` (internal capacity, copyable).
- **covered** by unit tests and continuous integration.
- **header only**, just include `function.hpp` in your project.

## Table of Contents

* **[Documentation](#documentation)**
  * **[Scheduling Tasks](#scheduling-tasks)**
* **[Compatibility](#compatibility)**
* **[License](#licence)**

## Documentation

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

`fu2::function`, `fu2::unique_function` and `std::function` are convertible to each other when

- The return type and parameter type match.
- The functions are both volatile or not.
- The functions are const correct:
  - `noconst = const`
  - `const = const`
  - `noconst = noconst`
- The functions are copyable correct
  - `unique = copyable`
  - `copyable = copyable`
  - | From \ To | `fu2::function` | `fu2::unique_function` | `std::function`
    | --
    | `fu2::function` | Yes | Yes | Yes
    | `fu2::unique_function` | No | Yes | No
    | `std::function` | Yes | Yes | Yes

```c++
fu2::function<void()> fun = []{};
std::function<void()> std_fun = fun;
fu2::unique_function<void()> un_fun = fun;

fun = std_fun;
```


## Compatibility

Tested with:

- Visual Studio 2015
- Clang 3.6+
- GCC 4.9.2+

Every compiler with full C++14 support should work (`constexpr` excluded for msvc).

## License

