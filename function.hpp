
//  Copyright 2015 Denis Blank <denis.blank at outlook dot com>
//   Distributed under the Boost Software License, Version 1.0
//      (See accompanying file LICENSE_1_0.txt or copy at
//           http://www.boost.org/LICENSE_1_0.txt)

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
    copyable() = default;
    copyable(copyable const&) = delete;
    copyable(copyable&&) = default;
    copyable& operator=(copyable const&) = delete;
    copyable& operator=(copyable&&) = default;
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

    /// Returns the size of the unique implementation
    virtual std::size_t size_unique() const = 0;

    /// Placed unique move
    virtual void move_unique_inplace(call_wrapper_interface* ptr) = 0;

}; // struct call_wrapper_interface

/// Interface: copyable wrapper
template<typename ReturnType, typename... Args, bool Constant, bool Volatile>
struct call_wrapper_interface<ReturnType(Args...), true, Constant, Volatile>
    // Inherit the non copyable base struct
    : call_wrapper_interface<ReturnType(Args...), false, Constant, Volatile>
{
    virtual ~call_wrapper_interface() { }

    /// Returns the size of the copyable implementation
    virtual std::size_t size_copyable() const = 0;

    /// Placed clone move
    virtual void move_copyable_inplace(call_wrapper_interface* ptr) = 0;

    /// Placed clone
    virtual void clone_copyable_inplace(call_wrapper_interface* ptr) const = 0;

    /// Allocate clone
    virtual call_wrapper_interface* clone_heap() const = 0;

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

    std::decay_t<T> _impl;

    call_wrapper_implementation() = delete;
    call_wrapper_implementation(call_wrapper_implementation const&) = delete;
    call_wrapper_implementation(call_wrapper_implementation&&) = delete;
    call_wrapper_implementation& operator= (call_wrapper_implementation const&) = delete;
    call_wrapper_implementation& operator= (call_wrapper_implementation&&) = delete;

    template<typename A>
    call_wrapper_implementation(A&& impl)
        : _impl(std::forward<A>(impl))
    {
        /*static_assert(!std::is_rvalue_reference<A>::value,
            "Can't create a non copyable wrapper with non r-value ref!");*/
    }

    virtual ~call_wrapper_implementation() { }

    std::size_t size_unique() const override
    {
        return sizeof(call_wrapper_implementation);
    }

    void move_unique_inplace(call_wrapper_interface<ReturnType(Args...), false, Constant, Volatile>* /*ptr*/) override
    {
        // new /*(ptr)*/ call_wrapper_implementation(std::move(_impl));
    }

    using call_virtual_operator<call_wrapper_implementation, ReturnType(Args...), false, Constant, Volatile>::operator();

}; // struct call_wrapper_implementation

/// Implementation: copyable wrapper
template<typename T, typename ReturnType, typename... Args, bool Constant, bool Volatile>
struct call_wrapper_implementation<T, ReturnType(Args...), true, Constant, Volatile>
     : call_virtual_operator<call_wrapper_implementation<T, ReturnType(Args...), true, Constant, Volatile>, ReturnType(Args...), true, Constant, Volatile>
{
    friend struct call_virtual_operator<call_wrapper_implementation, ReturnType(Args...), true, Constant, Volatile>;

    std::decay_t<T> _impl;

    call_wrapper_implementation() = delete;
    call_wrapper_implementation(call_wrapper_implementation const&) = delete;
    call_wrapper_implementation(call_wrapper_implementation&&) = delete;
    call_wrapper_implementation& operator= (call_wrapper_implementation const&) = delete;
    call_wrapper_implementation& operator= (call_wrapper_implementation&&) = delete;

    template<typename A>
    call_wrapper_implementation(A&& impl)
        : _impl(std::forward<A>(impl)) { }

    virtual ~call_wrapper_implementation() { }

    std::size_t size_unique() const override
    {
        return sizeof(call_wrapper_implementation<T, ReturnType(Args...), false, Constant, Volatile>);
    }

    std::size_t size_copyable() const override
    {
        return sizeof(call_wrapper_implementation);
    }

    void move_unique_inplace(call_wrapper_interface<ReturnType(Args...), false, Constant, Volatile>* ptr) override
    {
        new (ptr) call_wrapper_implementation<T, ReturnType(Args...), false, Constant, Volatile>(std::move(_impl));
    }

    void move_copyable_inplace(call_wrapper_interface<ReturnType(Args...), true, Constant, Volatile>* ptr) override
    {
        new (ptr) call_wrapper_implementation(std::move(_impl));
    }

    void clone_copyable_inplace(call_wrapper_interface<ReturnType(Args...), true, Constant, Volatile>* ptr) const
    {
        new (ptr) call_wrapper_implementation(std::move(_impl));
    }

    call_wrapper_interface<ReturnType(Args...), true, Constant, Volatile>* clone_heap() const override
    {
        return new call_wrapper_implementation(_impl);
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
        return (*static_cast<Base*>(this)->_storage._impl)(std::forward<Args>(args)...);
    }
};

template<typename Base, typename ReturnType, typename... Args, bool Copyable>
struct call_operator<Base, ReturnType(Args...), Copyable, true, false>
{
    ReturnType operator()(Args... args) const
    {
        return (*static_cast<const Base*>(this)->_storage._impl)(std::forward<Args>(args)...);
    }
};

template<typename Base, typename ReturnType, typename... Args, bool Copyable>
struct call_operator<Base, ReturnType(Args...), Copyable, false, true>
{
    ReturnType operator()(Args... args) volatile
    {
        return (*static_cast<volatile Base*>(this)->_storage._impl)(std::forward<Args>(args)...);
    }
};

template<typename Base, typename ReturnType, typename... Args, bool Copyable>
struct call_operator<Base, ReturnType(Args...), Copyable, true, true>
{
    ReturnType operator()(Args... args) const volatile
    {
        return (*static_cast<const volatile Base*>(this)->_storage._impl)(std::forward<Args>(args)...);
    }
};

template<typename /*Fn*/>
struct storage_t;

template<typename ReturnType, typename... Args, std::size_t Capacity, bool Copyable, bool Constant, bool Volatile>
struct storage_t<function<ReturnType(Args...), Capacity, Copyable, Constant, Volatile>>
{
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
    using interface_holder_t = interface_t*;
    /*typename qualified_t<
        interface_t, Constant, Volatile
    >::type**/;

    interface_holder_t _impl;

    std::uint8_t _storage[Capacity];

    template<typename T>
    using is_local_allocateable =
        std::integral_constant<bool,
        sizeof(T) <= sizeof(uint8_t[Capacity])
        /*&& (std::alignment_of<uint8_t[Capacity]>::value % std::alignment_of<T>::value) == 0*/>;

    void weak_deallocate()
    {
        if (_impl == static_cast<void*>(&_storage))
            _impl->~interface_t();
        else if (!_impl)
            delete _impl;
    }

    storage_t()
        : _impl(nullptr) { }

    storage_t(storage_t const& right)
        : _impl() { }

    storage_t(storage_t&& right)
        : _impl()
    {
    }

    storage_t& operator= (storage_t const& right)
    {
        copy_assign(right);
        return *this;
    }

    storage_t& operator= (storage_t&& right)
    {
        move_assign(std::forward<storage_t>(right));
        return *this;
    }

    ~storage_t()
    {
        weak_deallocate();
    }

    /// Direct allocate (use capacity)
    template<typename T>
    auto allocate(T&& functor)
        -> std::enable_if_t<is_local_allocateable<implementation_t<std::decay_t<T>>>::value>
    {
        weak_deallocate();

        static_assert(sizeof(T) <= Capacity, "[Debug] Overflow!");

        new (&_storage) implementation_t<std::decay_t<T>>(std::forward<T>(functor));
        _impl = reinterpret_cast<interface_holder_t>(&_storage);
    }

    /*
    /// Heap allocate
    template<typename T>
    auto allocate(T&& functor)
        -> std::enable_if_t<!is_local_allocateable<typename implementation_t<T>>::value>
    {
        weak_deallocate();

        _impl = new T(std::forward<T>(functor));
    }*/

    // Copy assign
    template<bool RightConstant>
    void copy_assign(storage_t<function<ReturnType(Args...), 0UL, true, RightConstant, Volatile>> const& right)
    {
        /*
        weak_deallocate();

        if (right._impl)
            _impl = right._impl->clone();
        else
            _impl = nullptr;
            */
    }

    // Move assign
    template<bool RightCopyable, bool RightConstant>
    void move_assign(storage_t<function<ReturnType(Args...), 0UL, RightCopyable, RightConstant, Volatile>>&& right)
    {
        /*
        weak_deallocate();
        _impl = right._impl;
        right._impl = nullptr;
        */
    }

    void deallocate()
    {
        weak_deallocate();
        _impl = nullptr;
    }
};

template<typename ReturnType, typename... Args, bool Copyable, bool Constant, bool Volatile>
struct storage_t<function<ReturnType(Args...), 0UL, Copyable, Constant, Volatile>>
{
    void weak_deallocate()
    {
        if (_impl)
            delete _impl;
    }

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

    storage_t(storage_t const& right)
        : _impl((right._impl) ? right._impl->clone_heap() : nullptr) { }

    storage_t(storage_t&& right)
        : _impl(right._impl)
    {
        right._impl = nullptr;
    }

    storage_t& operator= (storage_t const& right)
    {
        copy_assign(right);
        return *this;
    }

    storage_t& operator= (storage_t&& right)
    {
        move_assign(std::forward<storage_t>(right));
        return *this;
    }

    ~storage_t()
    {
        weak_deallocate();
    }

    template<typename T>
    void allocate(T&& functor)
    {
        weak_deallocate();
        _impl = new implementation_t<std::decay_t<T>>(std::forward<T>(functor));
    }

    // Copy assign
    template<bool RightConstant>
    void copy_assign(storage_t<function<ReturnType(Args...), 0UL, true, RightConstant, Volatile>> const& right)
    {
        weak_deallocate();
        
        if (right._impl)
            _impl = right._impl->clone_heap();
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

    void deallocate()
    {
        weak_deallocate();
        _impl = nullptr;
    }
};

template<typename ReturnType, typename... Args, std::size_t Capacity, bool Copyable, bool Constant, bool Volatile>
class function<ReturnType(Args...), Capacity, Copyable, Constant, Volatile>
    : public call_operator<function<ReturnType(Args...), Capacity, Copyable, Constant, Volatile>, ReturnType(Args...), Copyable, Constant, Volatile>,
      public signature<ReturnType, Args...>,
      public copyable<Copyable>
{
    friend struct call_operator<function, ReturnType(Args...), Copyable, Constant, Volatile>;

    // Is a true type if the given function is copyable correct to this.
    template<bool RightCopyable>
    using is_copyable_correct_to_this =
        std::integral_constant<bool,
            Copyable == RightCopyable
        >;

    // Is a true type if the given function is constant correct to this.
    template<bool RightConstant>
    using is_constant_correct_to_this =
        std::integral_constant<bool,
            !(Constant && !RightConstant)
        >;

    // Is a true type if the given function is volatile correct to this.
    template<bool RightVolatile>
    using is_volatile_correct_to_this =
        std::integral_constant<bool,
            Volatile == RightVolatile
        >;

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
            is_constant_correct_to_this<unwrap_t<T>::is_const>::value &&
            is_volatile_correct_to_this<unwrap_t<T>::is_volatile>::value
        >;

    // Implementation storage
    storage_t<function> _storage;

    template<typename T>
    static auto lambda_wrap(T&& arg)
    {
        return [arg = std::forward<T>(arg)](Args&&... args)
        {
            return arg(std::forward<Args>(args)...);
        };
    }

public:
    function() = default;

    /// Copy construct
    template<std::size_t RightCapacity, bool RightConstant,
             typename = std::enable_if_t<is_constant_correct_to_this<RightConstant>::value>>
    explicit function(function<ReturnType(Args...), RightCapacity, true, RightConstant, Volatile> const& right)
    {
        _storage.copy_assign(right);
    }

    /// Move construct
    template<std::size_t RightCapacity, bool RightCopyable, bool RightConstant,
             typename = std::enable_if_t<is_constant_correct_to_this<RightConstant>::value &&
                        is_copyable_correct_to_this<RightCopyable>::value>>
    explicit function(function<ReturnType(Args...), RightCapacity, RightCopyable, RightConstant, Volatile>&& right)
    {
        _storage.move_assign(std::move(right));
    }

    /// Constructor taking a function pointer
    template<typename T, typename = std::enable_if_t<is_function_pointer_assignable_to_this<T>::value>, typename = void>
    function(T function_pointer)
        : function(lambda_wrap(function_pointer)) { }

    /// Constructor taking a functor
    template<typename T, typename = std::enable_if_t<is_functor_assignable_to_this<T>::value>>
    function(T functor)
    {
        _storage.allocate(std::forward<T>(functor));
    }

    explicit function(std::nullptr_t)
        : _storage() { }

    /// Copy assign
    template<std::size_t RightCapacity, bool RightConstant,
             typename = std::enable_if_t<is_constant_correct_to_this<RightConstant>::value>>
    function& operator= (function<ReturnType(Args...), RightCapacity, true, RightConstant, Volatile> const& right)
    {
        _storage.copy_assign(right);
        return *this;
    }

    /// Move assign
    template<std::size_t RightCapacity, bool RightCopyable, bool RightConstant,
             typename = std::enable_if_t<is_constant_correct_to_this<RightConstant>::value &&
                        is_copyable_correct_to_this<RightCopyable>::value>>
    function& operator= (function<ReturnType(Args...), RightCapacity, RightCopyable, RightConstant, Volatile>&& right)
    {
        _storage.move_assign(std::move(right));
        return *this;
    }

    /// Copy assign taking a function pointer
    template<typename T, typename = std::enable_if_t<is_function_pointer_assignable_to_this<T>::value>, typename = void>
    function& operator= (T function_pointer)
    {
        _storage.allocate(lambda_wrap(function_pointer));
        return *this;
    }

    /// Copy assign taking a functor
    template<typename T, typename = std::enable_if_t<is_functor_assignable_to_this<T>::value>>
    function& operator= (T functor)
    {
        _storage.allocate(std::forward<T>(functor));
        return *this;
    }

    function& operator= (std::nullptr_t)
    {
        _storage.deallocate();
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
