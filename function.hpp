
//              Copyright Denis Blank 2015
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef function_hpp__
#define function_hpp__

#include <tuple>
#include <type_traits>

namespace my
{

namespace detail
{

namespace unwrap_traits
{
    template<
        typename /*Function*/,
        typename /*DecayedFunction*/,
        bool /*IsMember*/,
        bool /*IsConst*/,
        bool /*IsVolatile*/>
    struct unwrap_trait_base;

    /// Unwrap base
    template<
        typename Function,
        typename ReturnType, typename... Args,
        bool IsMember,
        bool IsConst,
        bool IsVolatile>
    struct unwrap_trait_base<Function, ReturnType(Args...), IsMember, IsConst, IsVolatile>
    {
        /// The full type of the given function.
        using type = Function;

        ///  The decayed type of the function without qualifiers.
        using decayed_type = ReturnType(Args...);

        /// The return type of the function.
        using return_type = ReturnType;

        /// The argument types of the function as pack in std::tuple.
        using argument_type = std::tuple<Args...>;

        /// Is true if the given function is a member function.
        static constexpr bool is_member = IsMember;

        /// Is true if the given function is const.
        static constexpr bool is_const = IsConst;

        /// Is true if the given function is volatile.
        static constexpr bool is_volatile = IsVolatile;
    };

    /// Function unwrap trait
    template<typename /*Fn*/>
    struct unwrap_trait;

    /// Function
    template<typename ReturnType, typename... Args>
    struct unwrap_trait<ReturnType(Args...)>
        : unwrap_trait_base<
            ReturnType(Args...), ReturnType(Args...), false, true, false> { };

    /// Function pointer
    template<typename ReturnType, typename... Args>
    struct unwrap_trait<ReturnType(*)(Args...)>
        : unwrap_trait_base<
            ReturnType(Args...), ReturnType(Args...), false, false, false> { };

    /// Const function pointer
    template<typename ReturnType, typename... Args>
    struct unwrap_trait<ReturnType(*const)(Args...)>
        : unwrap_trait_base<
            ReturnType(Args...) const, ReturnType(Args...), false, true, false> { };

    /// Class method pointer
    template<typename _CTy, typename ReturnType, typename... Args>
    struct unwrap_trait<ReturnType(_CTy::*)(Args...)>
        : unwrap_trait_base<
            ReturnType(Args...), ReturnType(Args...), true, false, false> { };

    /// Const class method pointer
    template<typename _CTy, typename ReturnType, typename... Args>
    struct unwrap_trait<ReturnType(_CTy::*)(Args...) const>
        : unwrap_trait_base<
            ReturnType(Args...) const, ReturnType(Args...), true, true, false> { };

} // namespace unwrap_traits

// type erasure call wrapper
template<typename>
struct wrapper_impl;

template<typename ReturnType, typename... Args>
struct wrapper_impl<ReturnType(Args...)>
{
    wrapper_impl() { }
    virtual ~wrapper_impl() { }

    virtual ReturnType operator() (Args&&...) = 0;

}; // struct wrapper_impl

template<typename /*Fn*/, bool /*NonCopyable*/, bool /*Constant*/, bool /*Volatile*/>
class function;

template<typename ReturnType, typename... Args, bool NonCopyable, bool Constant, bool Volatile>
class function<ReturnType(Args...), NonCopyable, Constant, Volatile>
{
    std::unique_ptr<wrapper_impl<ReturnType(Args...)>> _impl;

public:
    function()
        : _impl() { }

    auto operator() (Args&&... /*args*/)
        -> std::enable_if_t<!Constant && !Volatile, void /*ReturnType*/>
    {
        // return _impl(std::forward<Args>(args)...);
    }

}; // class function

} // namespace detail

template<typename Signature>
using function = detail::function<
    Signature,
    true /*NonCopyable*/,
    false /*Constant*/,
    false /*Volatile*/
>;

/// Creates a function object from the given parameter
template<typename Fn>
auto make_function(Fn&& functional)
{
    // TODO
    return std::forward<Fn>(functional);
}

} // namespace my

/*
template<typename Fn>
struct function;

template<typename ReturnType, typename... Args>
struct function<ReturnType(Args...)>
    : impl_function<ReturnType(Args...), true, false, false>
{
    // using impl_function<ReturnType(Args...), true, false, false>::impl_function;
};

template<typename ReturnType, typename... Args>
struct function<ReturnType(Args...) const>
    : impl_function<ReturnType(Args...), true, true, false>
{
};

template<typename ReturnType, typename... Args>
struct function<ReturnType(Args...) volatile>
    : impl_function<ReturnType(Args...), true, false, true> { };

template<typename ReturnType, typename... Args>
struct function<ReturnType(Args...) const volatile>
    : impl_function<ReturnType(Args...), true, true, true> { };


*/

#endif // function_hpp__
