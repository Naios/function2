
//              Copyright Denis Blank 2015
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef function_hpp__
#define function_hpp__

#include <tuple>
#include <memory>
#include <type_traits>

namespace my
{

namespace detail
{

namespace unwrap_traits
{
    template<
        typename /*DecayedFunction*/,
        bool /*IsMember*/,
        bool /*IsConst*/,
        bool /*IsVolatile*/>
    struct unwrap_trait_base;

    /// Unwrap base
    template<
        typename ReturnType, typename... Args,
        bool IsMember,
        bool IsConst,
        bool IsVolatile>
    struct unwrap_trait_base<ReturnType(Args...), IsMember, IsConst, IsVolatile>
    {
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

    template<typename ClassType>
    struct class_trait_base
    {
        /// Class type of the given function.
        using class_type = ClassType;
    };

    /// Function unwrap trait
    template<typename /*Fn*/>
    struct unwrap;

    /// Function
    template<typename ReturnType, typename... Args>
    struct unwrap<ReturnType(Args...)>
        : unwrap_trait_base<ReturnType(Args...), false, false, false> { };

    /// Const function
    template<typename ReturnType, typename... Args>
    struct unwrap<ReturnType(Args...) const>
        : unwrap_trait_base<ReturnType(Args...), false, true, false> { };

    /// Volatile function
    template<typename ReturnType, typename... Args>
    struct unwrap<ReturnType(Args...) volatile>
        : unwrap_trait_base<ReturnType(Args...), false, false, true> { };

    /// Const volatile function
    template<typename ReturnType, typename... Args>
    struct unwrap<ReturnType(Args...) const volatile>
        : unwrap_trait_base<ReturnType(Args...), false, true, true> { };

    /// Function pointer
    template<typename ReturnType, typename... Args>
    struct unwrap<ReturnType(*)(Args...)>
        : unwrap_trait_base<ReturnType(Args...), false, false, false> { };

    /// Const function pointer
    template<typename ReturnType, typename... Args>
    struct unwrap<ReturnType(*const)(Args...)>
        : unwrap_trait_base<ReturnType(Args...), false, true, false> { };

    /// Volatile function pointer
    template<typename ReturnType, typename... Args>
    struct unwrap<ReturnType(*volatile)(Args...)>
        : unwrap_trait_base<ReturnType(Args...), false, false, true> { };

    /// Const volatile function pointer
    template<typename ReturnType, typename... Args>
    struct unwrap<ReturnType(*const volatile)(Args...) >
        : unwrap_trait_base<ReturnType(Args...), false, true, true> { };

    /// Function pointer as reference
    template<typename ReturnType, typename... Args>
    struct unwrap<ReturnType(*&)(Args...)>
        : unwrap_trait_base<ReturnType(Args...), false, false, false> { };

    /// Const function pointer as reference
    template<typename ReturnType, typename... Args>
    struct unwrap<ReturnType(*const&)(Args...)>
        : unwrap_trait_base<ReturnType(Args...), false, true, false> { };

    /// Volatile function pointer as reference
    template<typename ReturnType, typename... Args>
    struct unwrap<ReturnType(*volatile&)(Args...)>
        : unwrap_trait_base<ReturnType(Args...), false, false, true> { };

    /// Const volatile function pointer as reference
    template<typename ReturnType, typename... Args>
    struct unwrap<ReturnType(*const volatile&)(Args...) >
        : unwrap_trait_base<ReturnType(Args...), false, true, true> { };

    /// Class method pointer
    template<typename ClassType, typename ReturnType, typename... Args>
    struct unwrap<ReturnType(ClassType::*)(Args...)>
        : unwrap_trait_base<ReturnType(Args...), true, false, false>,
          class_trait_base<ClassType> { };

    /// Const class method pointer
    template<typename ClassType, typename ReturnType, typename... Args>
    struct unwrap<ReturnType(ClassType::*)(Args...) const>
        : unwrap_trait_base<ReturnType(Args...), true, true, false>,
          class_trait_base<ClassType> { };

    /// Volatile class method pointer
    template<typename ClassType, typename ReturnType, typename... Args>
    struct unwrap<ReturnType(ClassType::*)(Args...) volatile>
        : unwrap_trait_base<ReturnType(Args...), true, false, true>,
          class_trait_base<ClassType> { };

    /// Const volatile class method pointer
    template<typename ClassType, typename ReturnType, typename... Args>
    struct unwrap<ReturnType(ClassType::*)(Args...) const volatile>
        : unwrap_trait_base<ReturnType(Args...), true, true, true>,
          class_trait_base<ClassType> { };

    /// 0. Unwrap classes through function pointer to operator()
    template<typename Fn>
    static constexpr auto do_unwrap(bool)
        -> unwrap<decltype(&Fn::operator())>;

    /// 1. Unwrap through plain type (function pointer)
    template<typename Fn>
    static constexpr auto do_unwrap(int)
        -> unwrap<Fn>;

    /// 2. Return std::false_type on failure.
    template<typename /*Fn*/>
    static constexpr auto do_unwrap(long)
        -> std::false_type;

    // Set unwrap_t to std::false_type if the given argument is not unwrappable,
    // to catch bad usage early.
    template<typename Fn>
    using unwrap_t = decltype(do_unwrap<Fn>(true));

} // namespace unwrap_traits

template<typename /*Fn*/>
struct move_only_wrapper_impl;

template<typename ReturnType, typename... Args>
struct move_only_wrapper_impl<ReturnType(Args...)>
{
    virtual ~move_only_wrapper_impl() { }

    virtual ReturnType operator() (Args&&...) = 0;

}; // struct move_only_wrapper_impl

template<typename /*Fn*/>
struct wrapper_impl;

template<typename ReturnType, typename... Args>
struct wrapper_impl<ReturnType(Args...)>
    : move_only_wrapper_impl<ReturnType(Args...)>
{
    virtual ~wrapper_impl() { }

    // virtual wrapper_impl* clone() = 0;

}; // struct wrapper_impl

template<typename /*Fn*/>
struct fake_wrapper_impl;

template<typename ReturnType, typename... Args>
struct fake_wrapper_impl<ReturnType(Args...)>
    : wrapper_impl<ReturnType(Args...)>
{
    ReturnType operator() (Args&&...) override
    {
        return ReturnType();
    }

}; // struct fake_wrapper_impl

template<typename /*Fn*/, bool /*Copyable*/, bool /*Constant*/, bool /*Volatile*/>
class function;

template <typename /*Child*/>
struct call_operator;

template <typename ReturnType, typename... Args, bool Copyable>
struct call_operator<function<ReturnType(Args...), Copyable, false, false>>
{
    using func = function<ReturnType(Args...), Copyable, false, false>;

    ReturnType operator()(Args... args)
    {
        return (*static_cast<func*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template <typename ReturnType, typename... Args, bool Copyable>
struct call_operator<function<ReturnType(Args...), Copyable, true, false>>
{
    using func = function<ReturnType(Args...), Copyable, true, false>;

    ReturnType operator()(Args... /*args*/) const
    {
        return ReturnType();
        // FIXME return (*static_cast<const func*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template <typename ReturnType, typename... Args, bool Copyable>
struct call_operator<function<ReturnType(Args...), Copyable, false, true>>
{
    using func = function<ReturnType(Args...), Copyable, false, true>;

    ReturnType operator()(Args... /*args*/) volatile
    {
        return ReturnType();
        // FIXME return (*static_cast<volatile func*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template <typename ReturnType, typename... Args, bool Copyable>
struct call_operator<function<ReturnType(Args...), Copyable, true, true>>
{
    using func = function<ReturnType(Args...), Copyable, true, true>;

    ReturnType operator()(Args... /*args*/) const volatile
    {
        return ReturnType();
        // FIXME return (*static_cast<const volatile func*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template<typename ReturnType, typename... Args, bool Copyable, bool Constant, bool Volatile>
class function<ReturnType(Args...), Copyable, Constant, Volatile>
    : public call_operator<function<ReturnType(Args...), Copyable, Constant, Volatile>>
{
    std::unique_ptr<wrapper_impl<ReturnType(Args...)>> _impl;

    friend struct call_operator<function>;

public:
    function()
        : _impl(std::make_unique<fake_wrapper_impl<ReturnType(Args...)>>()) { }

    template<typename T>
    function(T&&)
        : _impl(std::make_unique<fake_wrapper_impl<ReturnType(Args...)>>()) { }

    using call_operator<function>::operator();    

}; // class function

template<typename Signature, bool Copyable>
using function_base = function<
    typename unwrap_traits::unwrap<Signature>::decayed_type,
    Copyable,
    unwrap_traits::unwrap<Signature>::is_const,
    unwrap_traits::unwrap<Signature>::is_volatile
>;

} // namespace detail

/// Copyable function wrapper
template<typename Signature>
using function = detail::function_base<Signature, true>;

/// Non copyable function wrapper
template<typename Signature>
using non_copyable_function = detail::function_base<Signature, false>;

/// Creates a functional object from the given argument.
template<typename Fn>
auto make_function(Fn&& functional)
{
    using unwrap_t = detail::unwrap_traits::unwrap_t<Fn>;

    // If you encounter the static assert here check if you pass a functor (class with non templated operator())
    // or function pointer as parameter.
    static_assert(!std::is_same<unwrap_t, std::false_type>::value,
         "The given argument is not a functor or function pointer which makes unwrapping impossible!");

    return detail::function<
        typename unwrap_t::decayed_type,
        // Check if the given argument is copyable in any way.
        std::is_copy_assignable<std::decay_t<Fn>>::value ||
        std::is_copy_constructible<std::decay_t<Fn>>::value,
        unwrap_t::is_const,
        unwrap_t::is_volatile
    >(std::forward<Fn>(functional));
}

} // namespace my

#endif // function_hpp__
