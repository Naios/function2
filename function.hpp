
//              Copyright Denis Blank 2015
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef function_hpp__
#define function_hpp__

#include <tuple>
#include <cstdint>
#include <type_traits>

namespace fu2
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
protected:
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

template<typename /*Fn*/, bool /*Constant*/, bool /*Volatile*/>
struct call_wrapper_operator_interface;

// Interfaces for non copyable wrapper:
// No qualifiers
template<typename ReturnType, typename... Args>
struct call_wrapper_operator_interface<ReturnType(Args...), false, false>
{
    virtual ~call_wrapper_operator_interface() { }

    virtual ReturnType operator() (Args&&...) = 0;

}; // struct call_wrapper_operator_interface

// Const qualifier
template<typename ReturnType, typename... Args>
struct call_wrapper_operator_interface<ReturnType(Args...), true, false>
{
    virtual ~call_wrapper_operator_interface() { }

    virtual ReturnType operator() (Args&&...) const = 0;

}; // struct call_wrapper_operator_interface

// Volatile qualifier
template<typename ReturnType, typename... Args>
struct call_wrapper_operator_interface<ReturnType(Args...), false, true>
{
    virtual ~call_wrapper_operator_interface() { }

    virtual ReturnType operator() (Args&&...) volatile = 0;

}; // struct call_wrapper_operator_interface

// Const volatile qualifier
template<typename ReturnType, typename... Args>
struct call_wrapper_operator_interface<ReturnType(Args...), true, true>
{
    virtual ~call_wrapper_operator_interface() { }

    virtual ReturnType operator() (Args&&...) const volatile = 0;

}; // struct call_wrapper_operator_interface

template<typename /*Fn*/, bool /*Copyable*/, bool /*Constant*/, bool /*Volatile*/>
struct call_wrapper_interface;

template<typename ReturnType, typename... Args, bool Constant, bool Volatile>
struct call_wrapper_interface<ReturnType(Args...), false, Constant, Volatile>
     : call_wrapper_operator_interface<ReturnType(Args...), Constant, Volatile>
{
    virtual ~call_wrapper_interface() { }

    // Returns the size of the implementation
    virtual std::size_t size() const = 0;

    /// Placed move
    // virtual void move(call_wrapper_interface* ptr) = 0;

    /// Allocate move
    // virtual call_wrapper_interface* move() = 0;
};

/// Interface: copyable wrapper
template<typename ReturnType, typename... Args, bool Constant, bool Volatile>
struct call_wrapper_interface<ReturnType(Args...), true, Constant, Volatile>
    // Inherit the non copyable base struct
    : call_wrapper_interface<ReturnType(Args...), false, Constant, Volatile>
{
    virtual ~call_wrapper_interface() { }

    /// Placed clone
    virtual void clone(call_wrapper_interface* ptr) const = 0;

    /// Allocate clone
    virtual call_wrapper_interface* clone() const = 0;

}; // struct call_wrapper_interface

template <typename /*Base*/, typename /*Fn*/, bool Copyable, bool /*Constant*/, bool /*Volatile*/>
struct call_virtual_operator;

template<typename Base, typename ReturnType, typename... Args, bool Copyable>
struct call_virtual_operator<Base, ReturnType(Args...), Copyable, false, false>
     : call_wrapper_interface<ReturnType(Args...), Copyable, false, false>
{
    virtual ~call_virtual_operator() { }

    ReturnType operator()(Args&&... args) override
    {
        return (static_cast<Base*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template<typename Base, typename ReturnType, typename... Args, bool Copyable>
struct call_virtual_operator<Base, ReturnType(Args...), Copyable, true, false>
     : call_wrapper_interface<ReturnType(Args...), Copyable, true, false>
{
    virtual ~call_virtual_operator() { }

    ReturnType operator()(Args&&... args) const override
    {
        return (static_cast<const Base*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template<typename Base, typename ReturnType, typename... Args, bool Copyable>
struct call_virtual_operator<Base, ReturnType(Args...), Copyable, false, true>
     : call_wrapper_interface<ReturnType(Args...), Copyable, false, true>
{
    virtual ~call_virtual_operator() { }

    ReturnType operator()(Args&&... args) volatile override
    {
        return (static_cast<volatile Base*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template<typename Base, typename ReturnType, typename... Args, bool Copyable>
struct call_virtual_operator<Base, ReturnType(Args...), Copyable , true, true>
     : call_wrapper_interface<ReturnType(Args...), Copyable, true, true>
{
    virtual ~call_virtual_operator() { }

    ReturnType operator()(Args&&... args) const volatile override
    {
        return (static_cast<const volatile Base*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template<typename /*T*/, typename /*Fn*/, bool /*Copyable*/, bool /*Constant*/, bool /*Volatile*/>
struct call_wrapper_implementation;

/// Implementation: move only wrapper
template<typename T, typename ReturnType, typename... Args, bool Constant, bool Volatile>
struct call_wrapper_implementation<T, ReturnType(Args...), false, Constant, Volatile>
     : call_virtual_operator<call_wrapper_implementation<T, ReturnType(Args...), false, Constant, Volatile>, ReturnType(Args...), false, Constant, Volatile>
{
    friend struct call_virtual_operator<call_wrapper_implementation, ReturnType(Args...), false, Constant, Volatile>;

    typename qualified_t<T, Constant, Volatile>::type _impl;

    call_wrapper_implementation(T&& impl)
        : _impl(std::forward<T>(impl))
    {
        static_assert(!std::is_rvalue_reference<T>::value,
            "Can't create a non copyable wrapper with non r-value ref!");
    }

    virtual ~call_wrapper_implementation() { }

    std::size_t size() const override
    {
        return sizeof(call_wrapper_implementation);
    }

    /*void move(call_wrapper_interface<ReturnType(Args...), false, Constant, Volatile>* ptr) override
    {
        new (ptr) call_wrapper_implementation(T(std::move(_impl)));
    }

    call_wrapper_interface<ReturnType(Args...), false, Constant, Volatile>* move() override
    {
        new call_wrapper_implementation(T(std::move(_impl)));
    }*/

    using call_virtual_operator<call_wrapper_implementation, ReturnType(Args...), false, Constant, Volatile>::operator();

}; // struct call_wrapper_implementation

/// Implementation: copyable wrapper
template<typename T, typename ReturnType, typename... Args, bool Constant, bool Volatile>
struct call_wrapper_implementation<T, ReturnType(Args...), true, Constant, Volatile>
     : call_virtual_operator<call_wrapper_implementation<T, ReturnType(Args...), true, Constant, Volatile>, ReturnType(Args...), true, Constant, Volatile>
{
    friend struct call_virtual_operator<call_wrapper_implementation, ReturnType(Args...), true, Constant, Volatile>;

    typename qualified_t<std::decay_t<T>, Constant, Volatile>::type _impl;

    call_wrapper_implementation(T&& impl)
        : _impl(std::forward<T>(impl)) { }

    virtual ~call_wrapper_implementation() { }

    std::size_t size() const override
    {
        return sizeof(call_wrapper_implementation);
    }

    /*void move(call_wrapper_interface<ReturnType(Args...), true, true, true>* ptr) override
    {
        new (ptr) call_wrapper_implementation(T(std::move(_impl)));
    }

    call_wrapper_interface<ReturnType(Args...), false, Constant, Volatile>* move() override
    {
        new call_wrapper_implementation(T(std::move(_impl)));
    }*/

    void clone(call_wrapper_interface<ReturnType(Args...), true, Constant, Volatile>* /*ptr*/) const override
    {
        // new (ptr) call_wrapper_implementation(T(_impl));
    }

    call_wrapper_interface<ReturnType(Args...), true, Constant, Volatile>* clone() const override
    {
        // return new call_wrapper_implementation(T(_impl));
        return nullptr;
    };

    using call_virtual_operator<call_wrapper_implementation, ReturnType(Args...), true, Constant, Volatile>::operator();

}; // struct call_wrapper_implementation

template<typename /*Fn*/, std::size_t /*Capacity*/, bool /*Copyable*/, bool /*Constant*/, bool /*Volatile*/>
class function;

template <typename /*Base*/, typename /*Fn*/, bool Copyable, bool /*Constant*/, bool /*Volatile*/>
struct call_operator;

template<typename Base, typename ReturnType, typename... Args, bool Copyable>
struct call_operator<Base, ReturnType(Args...), Copyable, false, false>
{
    ReturnType operator()(Args... args)
    {
        return (*static_cast<Base*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template<typename Base, typename ReturnType, typename... Args, bool Copyable>
struct call_operator<Base, ReturnType(Args...), Copyable, true, false>
{
    ReturnType operator()(Args... args) const
    {
        return (*static_cast<const Base*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template<typename Base, typename ReturnType, typename... Args, bool Copyable>
struct call_operator<Base, ReturnType(Args...), Copyable, false, true>
{
    ReturnType operator()(Args... args) volatile
    {
        return (*static_cast<volatile Base*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template<typename Base, typename ReturnType, typename... Args, bool Copyable>
struct call_operator<Base, ReturnType(Args...), Copyable, true, true>
{
    ReturnType operator()(Args... args) const volatile
    {
        return (*static_cast<const volatile Base*>(this)->_impl)(std::forward<Args>(args)...);
    }
};

template<typename /*Fn*/>
struct storage_t;

template<typename ReturnType, typename... Args, std::size_t Capacity, bool Copyable, bool Constant, bool Volatile>
class storage_t<function<ReturnType(Args...), Capacity, Copyable, Constant, Volatile>>
    : public storage_t<function<ReturnType(Args...), 0UL, Copyable, Constant, Volatile>>
{
    static_assert(Capacity != 0, "Not implemented yet!");

    using base_t = storage_t<
        function<ReturnType(Args...), 0UL, Copyable, Constant, Volatile>
    >;

protected:
    /*storage_t()
        : base_t() { }

    storage_t(typename base_t::interface_holder_t impl)
        : base_t(impl) { }

    ~storage_t()
    {
    }*/

    std::uint8_t _storage[Capacity];
};

template<typename ReturnType, typename... Args, bool Copyable, bool Constant, bool Volatile>
class storage_t<function<ReturnType(Args...), 0UL, Copyable, Constant, Volatile>>
{
    void weak_deallocate()
    {
        if (_impl)
            delete _impl;
    }

public:
    // Call wrapper interface
    using interface_t = call_wrapper_interface<
        ReturnType(Args...), Copyable, Constant, Volatile
    >;

    // Call wrapper implementation
    template<typename T>
    using implementation_t = call_wrapper_implementation<
        T, ReturnType(Args...), Copyable, Constant, Volatile
    >;

    // Call wrapper qualified storage
    using interface_holder_t = typename qualified_t<
        interface_t, Constant, Volatile
    >::type*;

    interface_holder_t _impl;

    storage_t()
        : _impl(nullptr) { }

    // Copy construct
    template<bool RightConstant>
    storage_t(storage_t<function<ReturnType(Args...), 0UL, true, RightConstant, Volatile>> const& right)
        : _impl(right._impl->clone()) { }

    // Move construct
    template<bool RightCopyable, bool RightConstant>
    storage_t(storage_t<function<ReturnType(Args...), 0UL, RightCopyable, RightConstant, Volatile>>&& right)
        : _impl(right._impl)
    {
        right._impl = nullptr;
    }

    template<typename T>
    storage_t(T&& functor)
        : _impl(new call_wrapper_implementation<T, ReturnType(Args...), Copyable, Constant, Volatile>(std::forward<T>(functor))) { }

    //   new typename implementation_t<T>

    ~storage_t()
    {
        weak_deallocate();
    }

    storage_t& operator= (storage_t const&) = delete;
    storage_t& operator= (storage_t&&) = delete;

    // Copy assign
    template<bool RightCopyable, bool RightConstant>
    void copy_assign(storage_t<function<ReturnType(Args...), 0UL, RightCopyable, RightConstant, Volatile>> const& right)
    {
        weak_deallocate();
        
        if (right._impl)
            _impl = right._impl->clone();
        else
            _impl = nullptr;
    }

    // Move assign
    template<bool RightCopyable, bool RightConstant>
    void move_assign(storage_t<function<ReturnType(Args...), 0UL, RightCopyable, RightConstant, Volatile>>&& right)
    {
        weak_deallocate();
        _impl = right._impl;
        right._impl = nullptr;
    }

    template<typename T>
    void allocate(T&& functor)
    {
        weak_deallocate();
        _impl = new implementation_t<T>(std::forward<T>(functor));
    }

    void deallocate()
    {
        weak_deallocate();
        _impl = nullptr;
    }
};

template<typename ReturnType, typename... Args, std::size_t Capacity, bool Copyable, bool Constant, bool Volatile>
class function<ReturnType(Args...), Capacity, Copyable, Constant, Volatile>
    : public storage_t<function<ReturnType(Args...), Capacity, Copyable, Constant, Volatile>>,
      public call_operator<function<ReturnType(Args...), Capacity, Copyable, Constant, Volatile>, ReturnType(Args...), Copyable, Constant, Volatile>,
      public signature<ReturnType, Args...>,
      public copyable<Copyable>
{
    friend struct call_operator<function, ReturnType(Args...), Copyable, Constant, Volatile>;

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
    template<typename RightReturnType, typename... RightArgs, std::size_t RightCapacity, bool RightCopyable, bool RightConstant,
             typename = std::enable_if_t<Copyable, RightReturnType>>
    function(function<RightReturnType(RightArgs...), RightCapacity, RightCopyable, RightConstant, Volatile> const& right)
        : storage_t<function>(right) { }

    /// Move construct
    template<typename RightReturnType, typename... RightArgs, std::size_t RightCapacity, bool RightCopyable, bool RightConstant>
    function(function<RightReturnType(RightArgs...), RightCapacity, RightCopyable, RightConstant, Volatile>&& right)
        : storage_t<function>(std::move(right)) { }

    /// Constructor taking a function pointer
    template<typename T, typename = std::enable_if_t<is_function_pointer_assignable_to_this<T>::value>, typename = void>
    function(T ptr)
        : function([ptr](Args&&... args) { return ptr(std::forward<Args>(args)...); }) { }

    /// Constructor taking a functor
    template<typename T, typename = std::enable_if_t<is_functor_assignable_to_this<T>::value>>
    function(T functor)
        : storage_t<function>(std::forward<T>(functor)) { }

    ~function() { }

    function& operator= (function const& right)
    {
        copy_assign(right);
        return *this;
    }

    /// Copy assign
    template<typename RightReturnType, typename... RightArgs, std::size_t RightCapacity, bool RightConstant,
             typename = std::enable_if_t<Copyable, RightReturnType>>
    function& operator= (function<RightReturnType(RightArgs...), RightCapacity, true, RightConstant, Volatile> const& right)
    {
        copy_assign(right);
        return *this;
    }

    function& operator= (function&& right)
    {
        move_assign(std::move(right));
        return *this;
    }

    /// Move assign
    template<typename RightReturnType, typename... RightArgs, std::size_t RightCapacity, bool RightCopyable, bool RightConstant, bool RightVolatile>
    function& operator= (function<RightReturnType(RightArgs...), RightCapacity, RightCopyable, RightConstant, RightVolatile>&& right)
    {
        move_assign(std::move(right));
        return *this;
    }

    using call_operator<function, ReturnType(Args...), Copyable, Constant, Volatile>::operator();    

}; // class function

static constexpr std::size_t default_capacity = 0UL;

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
    detail::default_capacity,
    true
>;

/// Non copyable function wrapper
template<typename Signature>
using unique_function = function_base<
    Signature,
    detail::default_capacity,
    false
>;

/// Creates a functional object which type depends on the given functor or function pointer.
/// The second template parameter can be used to adjust the capacity
/// for small functor optimization (in-place allocation for small objects).
template<typename Fn, std::size_t Capacity = detail::default_capacity>
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

} // namespace fu2

#endif // function_hpp__
