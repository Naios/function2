
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

inline namespace v0
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
    auto do_unwrap(int)
        -> unwrap<decltype(&Fn::operator())>;

    /// 1. Unwrap through plain type (function pointer)
    template<typename Fn>
    auto do_unwrap(long)
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
    auto test_functor(int)
        -> to_true<decltype(&T::operator())>;

    template<typename T>
    auto test_functor(...)
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

template<typename>
struct deduce_t
    : std::true_type { };

template<std::size_t>
struct deduce_sz
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

    /// Returns true if the unique implementation can be allocated in-place in the given region.
    virtual bool can_allocate_unique_inplace(std::size_t size) const = 0;

    /// Placed unique move
    virtual void move_unique_inplace(call_wrapper_interface* ptr) = 0;

    /// Move the unique implementation to the heap
    virtual call_wrapper_interface* move_unique_to_heap() = 0;

}; // struct call_wrapper_interface

/// Interface: copyable wrapper
template<typename ReturnType, typename... Args, bool Constant, bool Volatile>
struct call_wrapper_interface<ReturnType(Args...), true, Constant, Volatile>
    // Inherit the non copyable base struct
     : call_wrapper_interface<ReturnType(Args...), false, Constant, Volatile>
{
    virtual ~call_wrapper_interface() { }

    /// Returns true if the copyable implementation can be allocated in-place in the given region.
    virtual bool can_allocate_copyable_inplace(std::size_t size) const = 0;

    /// Placed clone move
    virtual void move_copyable_inplace(call_wrapper_interface* ptr) = 0;

    /// Move the copyable implementation to the heap
    virtual call_wrapper_interface* move_copyable_to_heap() = 0;

    /// Placed clone to copyable
    virtual void clone_copyable_inplace(call_wrapper_interface* ptr) const = 0;

    /// Placed clone to unique
    virtual void clone_unique_inplace(call_wrapper_interface<ReturnType(Args...), false, Constant, Volatile>* ptr) const = 0;

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

}; // struct call_virtual_operator

template<typename Base, typename ReturnType, typename... Args, bool Copyable>
struct call_virtual_operator<Base, ReturnType(Args...), Copyable, true, false>
     : call_wrapper_interface<ReturnType(Args...), Copyable, true, false>
{
    virtual ~call_virtual_operator() { }

    ReturnType operator()(Args&&... args) const override
    {
        return (static_cast<const Base*>(this)->_impl)(std::forward<Args>(args)...);
    }

}; // struct call_virtual_operator

template<typename Base, typename ReturnType, typename... Args, bool Copyable>
struct call_virtual_operator<Base, ReturnType(Args...), Copyable, false, true>
     : call_wrapper_interface<ReturnType(Args...), Copyable, false, true>
{
    virtual ~call_virtual_operator() { }

    ReturnType operator()(Args&&... args) volatile override
    {
        return (static_cast<volatile Base*>(this)->_impl)(std::forward<Args>(args)...);
    }

}; // struct call_virtual_operator

template<typename Base, typename ReturnType, typename... Args, bool Copyable>
struct call_virtual_operator<Base, ReturnType(Args...), Copyable , true, true>
     : call_wrapper_interface<ReturnType(Args...), Copyable, true, true>
{
    virtual ~call_virtual_operator() { }

    ReturnType operator()(Args&&... args) const volatile override
    {
        return (static_cast<const volatile Base*>(this)->_impl)(std::forward<Args>(args)...);
    }

}; // struct call_virtual_operator

template<std::size_t Size, std::size_t Alignment>
using round_up_to_alignment = typename std::conditional<Size % Alignment == 0,
    std::integral_constant<std::size_t, Size>,
    std::integral_constant<std::size_t,
        // Rounds the required size up to the alignment
        Size + (Alignment - (Size % Alignment))
    >
>::type;

template<typename T>
using required_capacity_to_allocate_inplace = round_up_to_alignment<
    sizeof(T), std::alignment_of<T>::value
>;

/// Increases the chances when to fall back from in place to heap allocation for move performance.
using default_chances = std::integral_constant<std::size_t,
    2UL
>;

template<typename /*T*/, typename /*Fn*/, bool /*Copyable*/, bool /*Constant*/, bool /*Volatile*/, std::size_t Chance = default_chances::value>
struct call_wrapper_implementation;

template<typename /*T*/, typename /*Fn*/, bool /*Copyable*/, bool /*Constant*/, bool /*Volatile*/, std::size_t /*Chance*/>
struct generate_next_impl;

template<typename T, typename ReturnType, typename... Args, bool Copyable, bool Constant, bool Volatile, std::size_t Chance>
struct generate_next_impl<T, ReturnType(Args...), Copyable, Constant, Volatile, Chance>
{
    using type = call_wrapper_implementation<T, ReturnType(Args...), Copyable, Constant, Volatile, Chance - 1>;
};

template<typename T, typename ReturnType, typename... Args, bool Copyable, bool Constant, bool Volatile>
struct generate_next_impl<T, ReturnType(Args...), Copyable, Constant, Volatile, 0UL>
{
    using type = call_wrapper_implementation<T, ReturnType(Args...), Copyable, Constant, Volatile, 0UL>;
};

template <typename T, std::size_t Chance>
struct can_allocate_inplace_helper
{
    static bool can_allocate_inplace(std::size_t size)
    {
        return required_capacity_to_allocate_inplace<T>::value <= size;
    }
};

template <typename T>
struct can_allocate_inplace_helper<T, 0UL>
{
    static bool can_allocate_inplace(std::size_t /*size*/)
    {
        return false;
    }
};

template<typename T, typename Fn, bool Copyable, bool Constant, bool Volatile, std::size_t Chance>
using generate_next_impl_t = typename generate_next_impl<T, Fn, Copyable, Constant, Volatile, Chance>::type;

/// Implementation: move only wrapper
template<typename T, typename ReturnType, typename... Args, bool Constant, bool Volatile, std::size_t Chance>
struct call_wrapper_implementation<T, ReturnType(Args...), false, Constant, Volatile, Chance>
     : call_virtual_operator<call_wrapper_implementation<T, ReturnType(Args...), false, Constant, Volatile, Chance>, ReturnType(Args...), false, Constant, Volatile>
{
    friend struct call_virtual_operator<call_wrapper_implementation, ReturnType(Args...), false, Constant, Volatile>;

    T _impl;

    call_wrapper_implementation() = delete;
    call_wrapper_implementation(call_wrapper_implementation const&) = delete;
    call_wrapper_implementation(call_wrapper_implementation&&) = delete;
    call_wrapper_implementation& operator= (call_wrapper_implementation const&) = delete;
    call_wrapper_implementation& operator= (call_wrapper_implementation&&) = delete;

    template<typename A>
    call_wrapper_implementation(A&& impl)
        : _impl(std::forward<A>(impl)) { }

    virtual ~call_wrapper_implementation() { }

    bool can_allocate_unique_inplace(std::size_t size) const override
    {
        return can_allocate_inplace_helper<
            generate_next_impl_t<T, ReturnType(Args...), false, Constant, Volatile, Chance>, Chance
        >::can_allocate_inplace(size);
    }

    void move_unique_inplace(call_wrapper_interface<ReturnType(Args...), false, Constant, Volatile>* ptr) override
    {
        new (ptr) generate_next_impl_t<T, ReturnType(Args...), false, Constant, Volatile, Chance>(std::move(_impl));
    }

    call_wrapper_interface<ReturnType(Args...), false, Constant, Volatile>* move_unique_to_heap() override
    {
        return new call_wrapper_implementation(std::move(_impl));
    }

    using call_virtual_operator<call_wrapper_implementation, ReturnType(Args...), false, Constant, Volatile>::operator();

}; // struct call_wrapper_implementation

/// Implementation: copyable wrapper
template<typename T, typename ReturnType, typename... Args, bool Constant, bool Volatile, std::size_t Chance>
struct call_wrapper_implementation<T, ReturnType(Args...), true, Constant, Volatile, Chance>
     : call_virtual_operator<call_wrapper_implementation<T, ReturnType(Args...), true, Constant, Volatile, Chance>, ReturnType(Args...), true, Constant, Volatile>
{
    friend struct call_virtual_operator<call_wrapper_implementation, ReturnType(Args...), true, Constant, Volatile>;

    T _impl;

    call_wrapper_implementation() = delete;
    call_wrapper_implementation(call_wrapper_implementation const&) = delete;
    call_wrapper_implementation(call_wrapper_implementation&&) = delete;
    call_wrapper_implementation& operator= (call_wrapper_implementation const&) = delete;
    call_wrapper_implementation& operator= (call_wrapper_implementation&&) = delete;

    template<typename A>
    call_wrapper_implementation(A&& impl)
        : _impl(std::forward<A>(impl)) { }

    virtual ~call_wrapper_implementation() { }

    bool can_allocate_unique_inplace(std::size_t size) const override
    {
        return can_allocate_inplace_helper<
            generate_next_impl_t<T, ReturnType(Args...), false, Constant, Volatile, Chance>, Chance
        >::can_allocate_inplace(size);
    }

    bool can_allocate_copyable_inplace(std::size_t size) const override
    {
        return can_allocate_inplace_helper<
            generate_next_impl_t<T, ReturnType(Args...), true, Constant, Volatile, Chance>, Chance
        >::can_allocate_inplace(size);
    }

    void move_unique_inplace(call_wrapper_interface<ReturnType(Args...), false, Constant, Volatile>* ptr) override
    {
        // Downcast to unique impl
        new (ptr) generate_next_impl_t<T, ReturnType(Args...), false, Constant, Volatile, Chance>(std::move(_impl));
    }

    void move_copyable_inplace(call_wrapper_interface<ReturnType(Args...), true, Constant, Volatile>* ptr) override
    {
        new (ptr) generate_next_impl_t<T, ReturnType(Args...), true, Constant, Volatile, Chance>(std::move(_impl));
    }

    call_wrapper_interface<ReturnType(Args...), false, Constant, Volatile>* move_unique_to_heap() override
    {
        // Downcast to unique impl
        return new call_wrapper_implementation<T, ReturnType(Args...), false, Constant, Volatile>(std::move(_impl));
    }

    call_wrapper_interface<ReturnType(Args...), true, Constant, Volatile>* move_copyable_to_heap() override
    {
        return new call_wrapper_implementation(std::move(_impl));
    }

    void clone_unique_inplace(call_wrapper_interface<ReturnType(Args...), false, Constant, Volatile>* ptr) const override
    {
        new (ptr) call_wrapper_implementation<T, ReturnType(Args...), false, Constant, Volatile>(_impl);
    }

    void clone_copyable_inplace(call_wrapper_interface<ReturnType(Args...), true, Constant, Volatile>* ptr) const override
    {
        new (ptr) call_wrapper_implementation(_impl);
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

}; // struct call_operator

template<typename Base, typename ReturnType, typename... Args, bool Copyable>
struct call_operator<Base, ReturnType(Args...), Copyable, true, false>
{
    ReturnType operator()(Args... args) const
    {
        return (*static_cast<const Base*>(this)->_storage._impl)(std::forward<Args>(args)...);
    }

}; // struct call_operator

template<typename Base, typename ReturnType, typename... Args, bool Copyable>
struct call_operator<Base, ReturnType(Args...), Copyable, false, true>
{
    ReturnType operator()(Args... args) volatile
    {
        return (*static_cast<volatile Base*>(this)->_storage._impl)(std::forward<Args>(args)...);
    }

}; // struct call_operator

template<typename Base, typename ReturnType, typename... Args, bool Copyable>
struct call_operator<Base, ReturnType(Args...), Copyable, true, true>
{
    ReturnType operator()(Args... args) const volatile
    {
        return (*static_cast<const volatile Base*>(this)->_storage._impl)(std::forward<Args>(args)...);
    }

}; // struct call_operator

template<typename T, std::size_t Capacity>
struct storage_base_t
{
    storage_base_t() { }

    storage_base_t(T* impl)
        : _impl(impl) { }

    T* _impl;

    std::uint8_t _locale[Capacity];

    bool is_inplace() const
    {
        return _impl == static_cast<void const*>(&_locale);
    }

    void weak_deallocate()
    {
        if (is_inplace())
            _impl->~T();
        else if (_impl)
            delete _impl;
    }

}; // struct storage_base_t

template<typename T>
struct storage_base_t<T, 0UL>
{
    storage_base_t() { }

    storage_base_t(T* impl)
        : _impl(impl) { }

    T* _impl;

    bool is_inplace() const
    {
        return false;
    }

    void weak_deallocate()
    {
        if (_impl)
            delete _impl;
    }

}; // struct storage_base_t

template<typename /*Fn*/>
struct storage_t;

template<typename ReturnType, typename... Args, std::size_t Capacity, bool Copyable, bool Constant, bool Volatile>
struct storage_t<function<ReturnType(Args...), Capacity, Copyable, Constant, Volatile>>
    : public storage_base_t<call_wrapper_interface<ReturnType(Args...), Copyable, Constant, Volatile>, Capacity>
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

    // Storage base type
    using base_t = storage_base_t<
        interface_t, Capacity
    >;

    template<typename T>
    using is_local_allocateable =
        std::integral_constant<bool,
            required_capacity_to_allocate_inplace<T>::value <= Capacity>;

    storage_t()
        : base_t(nullptr) { }

    explicit storage_t(storage_t const& right) : base_t()
    {
        weak_copy_assign(right);
    }

    explicit storage_t(storage_t&& right) : base_t()
    {
        weak_move_assign(std::forward<storage_t>(right));
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
        this->weak_deallocate();
    }

    bool is_allocated() const
    {
        return this->_impl != nullptr;
    }

    void change_to_locale()
    {
        this->_impl = reinterpret_cast<interface_t*>(&this->_locale);
    }

    template<typename T>
    void allocate(T&& functor)
    {
        this->weak_deallocate();
        weak_allocate(std::forward<T>(functor));
    }

    /// Direct allocate (use capacity)
    template<typename T>
    auto weak_allocate(T&& functor)
        -> typename std::enable_if<is_local_allocateable<implementation_t<typename std::decay<T>::type>>::value>::type
    {
        new (&this->_locale) implementation_t<typename std::decay<T>::type>(std::forward<T>(functor));
        change_to_locale();
    }

    /// Heap allocate
    template<typename T>
    auto weak_allocate(T&& functor)
        -> typename std::enable_if<!is_local_allocateable<implementation_t<typename std::decay<T>::type>>::value>::type
    {
        this->_impl = new implementation_t<typename std::decay<T>::type>(std::forward<T>(functor));
    }

    template<typename T>
    void copy_assign(T const& right)
    {
        this->weak_deallocate();
        weak_copy_assign(right);
    }

    template<typename T>
    auto do_copy_allocate(T const& right)
        -> typename std::enable_if<Copyable && deduce_t<T>::value>::type
    {
        change_to_locale();
        right._impl->clone_copyable_inplace(this->_impl);
    }

    template<typename T>
    auto do_copy_allocate(T const& right)
        -> typename std::enable_if<!Copyable && deduce_t<T>::value>::type
    {
        change_to_locale();
        right._impl->clone_unique_inplace(this->_impl);
    }

    // Copy assign with in-place capability.
    template<std::size_t RightCapacity, bool RightConstant,
             typename std::enable_if<(Capacity > 0UL) && deduce_sz<RightCapacity>::value>::type* = nullptr>
    void weak_copy_assign(storage_t<function<ReturnType(Args...), RightCapacity, true, RightConstant, Volatile>> const& right)
    {
        if (!right.is_allocated())
            clean(); // Deallocate if right is unallocated
        else if (right._impl->can_allocate_copyable_inplace(Capacity))
            do_copy_allocate(right);
        else
            this->_impl = right._impl->clone_heap(); // heap clone
    }

    // Copy assign with no in-place capability.
    template<std::size_t RightCapacity, bool RightConstant,
             typename std::enable_if<(Capacity == 0UL) && deduce_sz<RightCapacity>::value>::type* = nullptr>
    void weak_copy_assign(storage_t<function<ReturnType(Args...), RightCapacity, true, RightConstant, Volatile>> const& right)
    {
        if (!right.is_allocated())
            clean(); // Deallocate if right is unallocated
        else
            this->_impl = right._impl->clone_heap(); // heap clone
    }

    template<typename T>
    static auto can_allocate_inplace(T const& right)
        -> typename std::enable_if<Copyable && deduce_t<T>::value, bool>::type
    {
        return right._impl->can_allocate_copyable_inplace(Capacity);
    }

    template<typename T>
    static auto can_allocate_inplace(T const& right)
        -> typename std::enable_if<!Copyable && deduce_t<T>::value, bool>::type
    {
        return right._impl->can_allocate_unique_inplace(Capacity);
    }

    template<typename T>
    auto do_move_allocate_inplace(T&& right)
        -> typename std::enable_if<Copyable && deduce_t<T>::value>::type
    {
        change_to_locale();
        right._impl->move_copyable_inplace(this->_impl);
        right.deallocate();
    }

    template<typename T>
    auto do_move_allocate_inplace(T&& right)
        -> typename std::enable_if<!Copyable && deduce_t<T>::value>::type
    {
        change_to_locale();
        right._impl->move_unique_inplace(this->_impl);
        right.deallocate();
    }

    template<typename T>
    auto do_move_allocate_to_heap(T&& right)
        -> typename std::enable_if<Copyable && deduce_t<T>::value>::type
    {
        this->_impl = right._impl->move_copyable_to_heap();
        right.deallocate();
    }

    template<typename T>
    auto do_move_allocate_to_heap(T&& right)
        -> typename std::enable_if<!Copyable && deduce_t<T>::value>::type
    {
        this->_impl = right._impl->move_unique_to_heap();
        right.deallocate();
    }

    template<typename T>
    void move_assign(T&& right)
    {
        this->weak_deallocate();
        weak_move_assign(std::forward<T>(right));
    }

    template<std::size_t RightCapacity, bool RightCopyable, bool RightConstant,
             typename std::enable_if<(Capacity > 0UL) && deduce_sz<RightCapacity>::value>::type* = nullptr>
    void weak_move_assign(storage_t<function<ReturnType(Args...), RightCapacity, RightCopyable, RightConstant, Volatile>>&& right)
    {
        if (!right.is_allocated())
            clean(); // Deallocate if right is unallocated
        else if (can_allocate_inplace(right))
            do_move_allocate_inplace(std::move(right));
        else
        {
            if (right.is_inplace())
                do_move_allocate_to_heap(std::move(right));
            else
            {
                this->_impl = right._impl;
                right._impl = nullptr;
            }
        }
    }

    template<std::size_t RightCapacity, bool RightCopyable, bool RightConstant,
             typename std::enable_if<(Capacity == 0UL) && deduce_sz<RightCapacity>::value>::type* = nullptr>
    void weak_move_assign(storage_t<function<ReturnType(Args...), RightCapacity, RightCopyable, RightConstant, Volatile>>&& right)
    {
        if (!right.is_allocated())
            clean(); // Deallocate if right is unallocated
        else
        {
            if (right.is_inplace())
                do_move_allocate_to_heap(std::move(right));
            else
            {
                this->_impl = right._impl;
                right._impl = nullptr;
            }
        }
    }

    void clean()
    {
        this->_impl = nullptr;
    }

    void deallocate()
    {
        this->weak_deallocate();
        clean();
    }

}; // struct storage_t

template<typename ReturnType, typename... Args, std::size_t Capacity, bool Copyable, bool Constant, bool Volatile>
class function<ReturnType(Args...), Capacity, Copyable, Constant, Volatile>
    : public call_operator<function<ReturnType(Args...), Capacity, Copyable, Constant, Volatile>, ReturnType(Args...), Copyable, Constant, Volatile>,
      public signature<ReturnType, Args...>,
      public copyable<Copyable>
{
    template<typename, std::size_t, bool, bool, bool>
    friend class function;

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

    // Box for small copyable types (function pointers)
    template<typename T>
    class functor_box
    {
        T _boxed;

    public:
        template<typename B>
        functor_box(B&& box)
            : _boxed(std::forward<B>(box)) { }

        ReturnType operator() (Args&&... args) const
        {
            return _boxed(std::forward<Args>(args)...);
        }
    };

    template<typename T>
    using functor_box_of = functor_box<typename std::decay<T>::type>;

public:
    function() = default;

    /// Copy construct
    template<std::size_t RightCapacity>
    explicit function(function<ReturnType(Args...), RightCapacity, true, Constant, Volatile> const& right)
    {
        _storage.weak_copy_assign(right._storage);
    }

    /// Move construct
    template<std::size_t RightCapacity, bool RightCopyable,
             typename = typename std::enable_if<is_copyable_correct_to_this<RightCopyable>::value>::type>
    explicit function(function<ReturnType(Args...), RightCapacity, RightCopyable, Constant, Volatile>&& right)
    {
        _storage.weak_move_assign(std::move(right._storage));
    }
    // typename std::enable_if<>::type* = nullptr
    /// Constructor taking a function pointer
    template<typename T, typename = typename std::enable_if<is_function_pointer_assignable_to_this<T>::value>::type, typename = void>
    function(T function_pointer)
        : function(functor_box_of<T>(std::forward<T>(function_pointer))) { }
    // typename std::enable_if<>::type* = nullptr
    /// Constructor taking a functor
    template<typename T, typename = typename std::enable_if<is_functor_assignable_to_this<T>::value>::type>
    function(T functor)
    {
        _storage.weak_allocate(std::forward<T>(functor));
    }

    explicit function(std::nullptr_t)
        : _storage() { }

    /// Copy assign
    template<std::size_t RightCapacity>
    function& operator= (function<ReturnType(Args...), RightCapacity, true, Constant, Volatile> const& right)
    {
        _storage.copy_assign(right._storage);
        return *this;
    }

    /// Move assign
    template<std::size_t RightCapacity, bool RightCopyable,
             typename std::enable_if<is_copyable_correct_to_this<RightCopyable>::value>::type* = nullptr>
    function& operator= (function<ReturnType(Args...), RightCapacity, RightCopyable, Constant, Volatile>&& right)
    {
        _storage.move_assign(std::move(right._storage));
        return *this;
    }

    /// Copy assign taking a function pointer
    template<typename T, typename std::enable_if<is_function_pointer_assignable_to_this<T>::value>::type* = nullptr>
    function& operator= (T function_pointer)
    {
        _storage.allocate(functor_box_of<T>(std::forward<T>(function_pointer)));
        return *this;
    }

    /// Copy assign taking a functor
    template<typename T, typename std::enable_if<is_functor_assignable_to_this<T>::value>::type* = nullptr>
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

// Default capacity for small functor optimization
using default_capacity = std::integral_constant<std::size_t,
    32UL
>;

} // inline namespace v0

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

namespace experimental
{

/// Creates a functional object which type depends on the given functor or function pointer.
/// The second template parameter can be used to adjust the capacity
/// for small functor optimization (in-place allocation for small objects).
/*template<typename Fn, std::size_t Capacity = detail::default_capacity::value>
auto make_function(Fn functional)
{
    static_assert(detail::is_function_pointer<Fn>::value || detail::is_functor<typename std::decay<Fn>::type>::value,
        "Can only create functions from functors and function pointers!");

    using unwrap_t = detail::unwrap_t<typename std::decay<Fn>::type>;

    return detail::function<
        typename unwrap_t::decayed_type,
        Capacity,
        // Check if the given argument is copyable in any way.
        std::is_copy_assignable<typename std::decay<Fn>::type>::value ||
        std::is_copy_constructible<typename std::decay<Fn>::type>::value,
        unwrap_t::is_const,
        unwrap_t::is_volatile
    >(std::forward<Fn>(functional));
}*/

} // namespace experimental

} // namespace fu2

#endif // function_hpp__
