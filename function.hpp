
//              Copyright Denis Blank 2015
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef function_hpp__
#define function_hpp__

#include <tuple>
#include <cstdint>
#include <type_traits>

namespace my
{

namespace detail
{

template<typename ReturnType, typename... Args>
struct signature
{
    /// The return type of the function.
    using return_type = ReturnType;

    /// The argument types of the function as pack in std::tuple.
    using argument_type = std::tuple<Args...>;
};

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
        : signature<ReturnType, Args...>
    {
        ///  The decayed type of the function without qualifiers.
        using decayed_type = ReturnType(Args...);

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
        : unwrap_trait_base<ReturnType(Args...), false, true, false> { };

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
    static constexpr auto do_unwrap(int)
        -> unwrap<decltype(&Fn::operator())>;

    /// 1. Unwrap through plain type (function pointer)
    template<typename Fn>
    static constexpr auto do_unwrap(long)
        -> unwrap<Fn>;

} // namespace unwrap_traits

/// Functional unwraps the given functional type
template<typename Fn>
using unwrap_t = decltype(unwrap_traits::do_unwrap<Fn>(0));

namespace is_functor_impl
{
    template<typename>
    struct to_true
        : std::true_type { };

    template<typename T>
    static inline constexpr auto test_functor(int)
        -> to_true<decltype(&T::operator())>;

    template<typename T>
    static inline constexpr auto test_functor(...)
        -> std::false_type;

} // namespace is_functor_impl

/// Is functor trait
template<typename T>
struct is_functor
    : decltype(is_functor_impl::test_functor<T>(0)) { };

/// Is function pointer trait
template<typename T>
struct is_function_pointer
    : std::false_type { };

template<typename ReturnType, typename... Args>
struct is_function_pointer<ReturnType(*)(Args...)>
    : std::true_type { };

template<bool>
struct copyable { };

template <>
struct copyable<false>
{
    copyable() { }

    copyable(copyable const&) = delete;
    copyable& operator=(copyable const&) = delete;
};

template<typename T, bool /*Constant*/, bool /*Volatile*/>
struct qualified_t
{
    using type = T;
};

template<typename T>
struct qualified_t<T, true, false>
{
    using type = T const;
};

template<typename T>
struct qualified_t<T, false, true>
{
    using type = T volatile;
};

template<typename T>
struct qualified_t<T, true, true>
{
    using type = T const volatile;
};

template <typename /*Base*/, typename /*Fn*/, bool /*Constant*/, bool /*Volatile*/, bool /*ForVirtualImpl*/>
struct call_operator;

template<typename Base, typename ReturnType, typename... Args>
struct call_operator<Base, ReturnType(Args...), false, false, false>
{
    ReturnType operator()(Args... args)
    {
        return (*static_cast<Base*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template<typename Base, typename ReturnType, typename... Args>
struct call_operator<Base, ReturnType(Args...), true, false, false>
{
    ReturnType operator()(Args... args) const
    {
        return (*static_cast<const Base*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template<typename Base, typename ReturnType, typename... Args>
struct call_operator<Base, ReturnType(Args...), false, true, false>
{
    ReturnType operator()(Args... args) volatile
    {
        return (*static_cast<volatile Base*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template<typename Base, typename ReturnType, typename... Args>
struct call_operator<Base, ReturnType(Args...), true, true, false>
{
    ReturnType operator()(Args... args) const volatile
    {
        return (*static_cast<const volatile Base*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template<typename Base, typename ReturnType, typename... Args>
struct call_operator<Base, ReturnType(Args...), false, false, true>
{
    ReturnType operator()(Args... args) override
    {
        return (static_cast<Base*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template<typename Base, typename ReturnType, typename... Args>
struct call_operator<Base, ReturnType(Args...), true, false, true>
{
    ReturnType operator()(Args... args) const override
    {
        return (static_cast<const Base*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template<typename Base, typename ReturnType, typename... Args>
struct call_operator<Base, ReturnType(Args...), false, true, true>
{
    ReturnType operator()(Args... args) volatile override
    {
        return (static_cast<volatile Base*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template<typename Base, typename ReturnType, typename... Args>
struct call_operator<Base, ReturnType(Args...), true, true, true>
{
    ReturnType operator()(Args... args) const volatile override
    {
        return (static_cast<const volatile Base*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template<typename /*Fn*/, bool /*Copyable*/, bool /*Constant*/, bool /*Volatile*/>
struct call_wrapper_interface;

// Interfaces for non copyable wrapper:
// No qualifiers
template<typename ReturnType, typename... Args>
struct call_wrapper_interface<ReturnType(Args...), false, false, false>
{
    virtual ~call_wrapper_interface() { }

    virtual ReturnType operator() (Args&&...) = 0;

}; // struct call_wrapper_interface

// Const qualifier
template<typename ReturnType, typename... Args>
struct call_wrapper_interface<ReturnType(Args...), false, true, false>
{
    virtual ~call_wrapper_interface() { }

    virtual ReturnType operator() (Args&&...) const = 0;

}; // struct call_wrapper_interface

// Volatile qualifier
template<typename ReturnType, typename... Args>
struct call_wrapper_interface<ReturnType(Args...), false, false, true>
{
    virtual ~call_wrapper_interface() { }

    virtual ReturnType operator() (Args&&...) volatile = 0;

}; // struct call_wrapper_interface

// Const volatile qualifier
template<typename ReturnType, typename... Args>
struct call_wrapper_interface<ReturnType(Args...), false, true, true>
{
    virtual ~call_wrapper_interface() { }

    virtual ReturnType operator() (Args&&...) const volatile = 0;

}; // struct call_wrapper_interface

/// Interface: copyable wrapper
template<typename ReturnType, typename... Args, bool Constant, bool Volatile>
struct call_wrapper_interface<ReturnType(Args...), true, Constant, Volatile>
     // Inherit the non copyable base struct
     : call_wrapper_interface<ReturnType(Args...), false, Constant, Volatile>
{
    virtual ~call_wrapper_interface() { }

    virtual call_wrapper_interface* clone() const = 0;

}; // struct call_wrapper_interface

template<typename /*T*/, typename /*Fn*/, bool /*Copyable*/, bool /*Constant*/, bool /*Volatile*/>
struct call_wrapper_implementation;

/// Implementation: move only wrapper
template<typename T, typename ReturnType, typename... Args, bool Constant, bool Volatile>
struct call_wrapper_implementation<T, ReturnType(Args...), false, Constant, Volatile>
     : call_wrapper_interface<ReturnType(Args...), false, Constant, Volatile>,
       call_operator<call_wrapper_implementation<T, ReturnType(Args...), false, Constant, Volatile>, ReturnType(Args...), Constant, Volatile, true>
{
    friend struct call_operator<call_wrapper_implementation, ReturnType(Args...), Constant, Volatile, true>;

    typename qualified_t<T, Constant, Volatile>::type _impl;

    virtual ~call_wrapper_implementation() { }

    using call_operator<call_wrapper_implementation, ReturnType(Args...), Constant, Volatile, true>::operator();

}; // struct call_wrapper_implementation

/// Implementation: copyable wrapper
template<typename T, typename ReturnType, typename... Args, bool Constant, bool Volatile>
struct call_wrapper_implementation<T, ReturnType(Args...), true, Constant, Volatile>
     : call_wrapper_interface<ReturnType(Args...), true, Constant, Volatile>,
       call_operator<call_wrapper_implementation<T, ReturnType(Args...), true, Constant, Volatile>, ReturnType(Args...), Constant, Volatile, true>
{
    friend struct call_operator<call_wrapper_implementation, ReturnType(Args...), Constant, Volatile, true>;

    typename qualified_t<T, Constant, Volatile>::type _impl;

    virtual ~call_wrapper_implementation() { }

    call_wrapper_interface<ReturnType(Args...), true, Constant, Volatile>* clone() const override
    {
        return nullptr;
    };

    using call_operator<call_wrapper_implementation, ReturnType(Args...), Constant, Volatile, true>::operator();

}; // struct call_wrapper_implementation

template<typename /*Fn*/, std::size_t /*Capacity*/, bool /*Copyable*/, bool /*Constant*/, bool /*Volatile*/>
class function;

template<typename /*Fn*/>
struct storage_t;

template<typename ReturnType, typename... Args, std::size_t Capacity, bool Copyable, bool Constant, bool Volatile>
class storage_t<function<ReturnType(Args...), Capacity, Copyable, Constant, Volatile>>
    : public storage_t<function<ReturnType(Args...), 0UL, Copyable, Constant, Volatile>>
{
    using base_t = storage_t<function<ReturnType(Args...), 0UL, Copyable, Constant, Volatile>>;

protected:
    storage_t()
        : base_t() { }

    storage_t(typename base_t::implementation_t impl)
        : base_t(impl) { }

    ~storage_t()
    {
    }

    std::uint8_t _storage[Capacity];
};

template<typename ReturnType, typename... Args, bool Copyable, bool Constant, bool Volatile>
class storage_t<function<ReturnType(Args...), 0UL, Copyable, Constant, Volatile>>
{
protected:
    using implementation_t = typename qualified_t<
        call_wrapper_interface<ReturnType(Args...), Copyable, Constant, Volatile>,
        Constant, Volatile
    >::type*;

    implementation_t _impl;

    storage_t()
        : _impl(nullptr) { }

    storage_t(implementation_t impl)
        : _impl(impl) { }
};

template<typename ReturnType, typename... Args, std::size_t Capacity, bool Copyable, bool Constant, bool Volatile>
class function<ReturnType(Args...), Capacity, Copyable, Constant, Volatile>
    : public storage_t<function<ReturnType(Args...), Capacity, Copyable, Constant, Volatile>>,
      public call_operator<function<ReturnType(Args...), Capacity, Copyable, Constant, Volatile>, ReturnType(Args...), Constant, Volatile, false>,
      public signature<ReturnType, Args...>,
      public copyable<Copyable>
{
    friend struct call_operator<function, ReturnType(Args...), Constant, Volatile, false>;

    // Is a true type if the given function pointer is assignable to this.
    template<typename T>
    using is_function_pointer_assignable_to_this =
        std::integral_constant<bool,
            is_function_pointer<T>::value &&
            !Volatile
        >;

    // Is a true type if the functor class is assignable to this.
    template<typename T>
    using is_functor_assignable_to_this =
        std::integral_constant<bool,
            is_functor<T>::value &&
            !(Constant && !unwrap_t<T>::is_const) &&
            (Volatile == unwrap_t<T>::is_volatile)
        >;

public:
    function()
        : storage_t<function>() { }

    /// Copy construct
    template<typename RightReturnType, typename... RightArgs, std::size_t RightCapacity, bool RightCopyable, bool RightConstant, bool RightVolatile,
             typename = std::enable_if_t<Copyable, RightReturnType>>
    function(function<RightReturnType(RightArgs...), RightCapacity, RightCopyable, RightConstant, RightVolatile> const& /*function*/)
        : storage_t<function>()
    {
        // TODO
    }

    /// Move construct
    template<typename RightReturnType, typename... RightArgs, std::size_t RightCapacity, bool RightCopyable, bool RightConstant, bool RightVolatile>
    function(function<RightReturnType(RightArgs...), RightCapacity, RightCopyable, RightConstant, RightVolatile>&& /*function*/)
        : storage_t<function>()
    {
        // TODO
    }

    /// Constructor taking a function pointer
    template<typename T, typename = std::enable_if_t<is_function_pointer_assignable_to_this<T>::value>, typename = void>
    function(T ptr)
        : function([ptr](Args&&... args) { return ptr(std::forward<Args>(args)...); }) { }

    /// Constructor taking a functor
    template<typename T, typename = std::enable_if_t<is_functor_assignable_to_this<T>::value>>
    function(T /*functor*/)
        : storage_t<function>()
    {
    }

    ~function()
    {
    }

    /// Copy assign
    template<typename RightReturnType, typename... RightArgs, std::size_t RightCapacity, bool RightCopyable, bool RightConstant, bool RightVolatile,
             typename = std::enable_if_t<Copyable, RightReturnType>>
    function& operator= (function<RightReturnType(RightArgs...), RightCapacity, RightCopyable, RightConstant, RightVolatile> const& /*right*/)
    {
        // TODO
        return *this;
    }

    /// Move assign
    template<typename RightReturnType, typename... RightArgs, std::size_t RightCapacity, bool RightCopyable, bool RightConstant, bool RightVolatile>
    function& operator= (function<RightReturnType(RightArgs...), RightCapacity, RightCopyable, RightConstant, RightVolatile>&& /*right*/)
    {
        // TODO
        return *this;
    }

    using call_operator<function, ReturnType(Args...), Constant, Volatile, false>::operator();    

}; // class function

struct default_capacity
    : std::integral_constant<std::size_t, 20UL> { };

} // namespace detail

/// Function wrapper base
template<typename Signature, std::size_t Capacity, bool Copyable>
using function_base = detail::function<
    typename detail::unwrap_traits::unwrap<Signature>::decayed_type,
    Capacity,
    Copyable,
    detail::unwrap_traits::unwrap<Signature>::is_const,
    detail::unwrap_traits::unwrap<Signature>::is_volatile
>;

/// Copyable function wrapper
template<typename Signature>
using function = function_base<
    Signature,
    detail::default_capacity::value,
    true
>;

/// Non copyable function wrapper
template<typename Signature>
using unique_function = function_base<
    Signature,
    detail::default_capacity::value,
    false
>;

/// Creates a functional object which type depends on the given functor or function pointer.
/// The second template parameter can be used to adjust the capacity
/// for small functor optimization (in-place allocation for small objects).
template<typename Fn, std::size_t Capacity = detail::default_capacity::value>
auto make_function(Fn functional)
{
    static_assert(detail::is_function_pointer<Fn>::value || detail::is_functor<std::decay_t<Fn>>::value,
        "Can only create functions from functors and function pointers!");

    using unwrap_t = detail::unwrap_t<std::decay_t<Fn>>;

    return detail::function<
        typename unwrap_t::decayed_type,
        Capacity,
        // Check if the given argument is copyable in any way.
        std::is_copy_assignable<std::decay_t<Fn>>::value ||
        std::is_copy_constructible<std::decay_t<Fn>>::value,
        unwrap_t::is_const,
        unwrap_t::is_volatile
    >(std::forward<Fn>(functional));
}

} // namespace my

#endif // function_hpp__
