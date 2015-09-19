
//  Copyright 2015 Denis Blank <denis.blank at outlook dot com>
//   Distributed under the Boost Software License, Version 1.0
//      (See accompanying file LICENSE_1_0.txt or copy at
//           http://www.boost.org/LICENSE_1_0.txt)

#ifndef fu2_included_function_hpp__
#define fu2_included_function_hpp__

#include <tuple>
#include <cstdint>
#include <type_traits>

namespace fu2
{

namespace detail
{

inline namespace v1
{

/// Type deducer
template<typename, typename T = std::true_type>
struct deduce_t
    : T { };

/// Size deducer
template<std::size_t, typename T = std::true_type>
struct deduce_sz
    : T { };

/// Tag for tag dispatching
template<bool>
struct tag { };

/// Copy enabler helper class
template<bool Copyable>
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

/// Helper to store function signature.
template<typename Signature>
struct signature;

template<typename ReturnType, typename... Args>
struct signature<ReturnType(Args...)>
{
    /// The function type
    using type = ReturnType(Args...);

    /// The return type of the function.
    using return_type = ReturnType;

    /// The argument types of the function as pack in std::tuple.
    using argument_type = std::tuple<Args...>;
};

/// Helper to store function qualifiers.
template<bool Constant, bool Volatile, bool RValue>
struct qualifier
{
    /// Is true if the qualifier has const.
    static constexpr bool is_const = Constant;

    /// Is true if the qualifier has volatile.
    static constexpr bool is_volatile = Volatile;

    /// Is true if the qualifier has r-value reference.
    static constexpr bool is_rvalue = RValue;
};

/// Helper to store the function configuration.
template<std::size_t Capacity, bool Copyable>
struct config
{
    /// The internal capacity of the function for sfo optimization.
    static constexpr std::size_t capacity = Capacity;

    /// Is true if the function is copyable.
    static constexpr bool is_copyable = Copyable;
};

// If macro.
#define FU2_MACRO_IF(cond) \
    FU2_MACRO_IF_ ## cond
#define FU2_MACRO_IF_true(EXPRESSION) EXPRESSION
#define FU2_MACRO_IF_false(EXPRESSION)

// If macro to turn the expression into a r-value expression.
#define FU2_MACRO_MOVE_IF(cond) \
    FU2_MACRO_MOVE_IF_ ## cond
#define FU2_MACRO_MOVE_IF_true(EXPRESSION) std::move( EXPRESSION )
#define FU2_MACRO_MOVE_IF_false(EXPRESSION) ( EXPRESSION )

// Qualifier without r-value ref.
#define FU2_MACRO_NO_REF_QUALIFIER(IS_CONST, IS_VOLATILE) \
    FU2_MACRO_IF(IS_CONST)(const) \
    FU2_MACRO_IF(IS_VOLATILE)(volatile)

// Full qualifier
#define FU2_MACRO_FULL_QUALIFIER(IS_CONST, IS_VOLATILE, IS_RVALUE) \
    FU2_MACRO_NO_REF_QUALIFIER(IS_CONST, IS_VOLATILE) \
    FU2_MACRO_IF(IS_RVALUE)(&&)

// Expand the given macro with all possible combinations.
#define FU2_MACRO_EXPAND_3(EXPRESSION) \
    EXPRESSION(false, false, false) \
    EXPRESSION(false, false, true) \
    EXPRESSION(false, true, false) \
    EXPRESSION(false, true, true) \
    EXPRESSION(true, false, false) \
    EXPRESSION(true, false, true) \
    EXPRESSION(true, true, false) \
    EXPRESSION(true, true, true)

template<typename Fn>
struct impl_is_callable_with_qualifiers;

template<typename ReturnType, typename... Args>
struct impl_is_callable_with_qualifiers<ReturnType(Args...)>
{
    template<typename T>
    static auto test(int)
        -> typename std::is_convertible<
            decltype(std::declval<T>()(std::declval<Args>()...)),
            ReturnType
           >;

    template<typename T>
    static auto test(...)
        -> std::false_type;
};

template<bool Condition, typename T>
using add_const_if_t = typename std::conditional<
    Condition,
    typename std::add_const<T>::type,
    T
>::type;

template<bool Condition, typename T>
using add_volatile_if_t = typename std::conditional<
    Condition,
    typename std::add_volatile<T>::type,
    T
>::type;

template<bool Condition, typename T>
using add_lvalue_if_t = typename std::conditional<
    Condition,
    typename std::add_lvalue_reference<T>::type,
    T
>::type;

template<typename T, typename Fn, typename Qualifier>
using is_callable_with_qualifiers = decltype(impl_is_callable_with_qualifiers<Fn>::template test<
    add_lvalue_if_t<!Qualifier::is_rvalue,
        add_volatile_if_t<Qualifier::is_volatile,
            add_const_if_t<Qualifier::is_const,
                typename std::decay<T>::type>>>
>(0));

// Is a true type if the left type is copyable correct to the right type.
template<bool LeftCopyable, bool RightCopyable>
using is_copyable_correct = std::integral_constant<bool,
    !(LeftCopyable && !RightCopyable)
>;

/// Function unwrap trait
template<typename Signature, typename Qualifier>
struct unwrap_base
{
    // The signature of the function
    using signature = Signature;

    // The qualifier of the function
    using qualifier = Qualifier;
};

/// Function unwrap trait
template<typename Fn>
struct unwrap
{
    static_assert(!deduce_t<Fn>::value,
        "Incompatible signature given, signature must be in the form of \"ReturnType(Arg...) Qualifier\".");
};

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...)>
    : unwrap_base<signature<ReturnType(Args...)>, qualifier<false, false, false>> { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...) const>
    : unwrap_base<signature<ReturnType(Args...)>, qualifier<true, false, false>> { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...) volatile>
    : unwrap_base<signature<ReturnType(Args...)>, qualifier<false, true, false>> { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...) const volatile>
    : unwrap_base<signature<ReturnType(Args...)>, qualifier<true, true, false>> { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...)&>
    : unwrap_base<signature<ReturnType(Args...)>, qualifier<false, false, false>> { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...) const&>
    : unwrap_base<signature<ReturnType(Args...)>, qualifier<true, false, false>> { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...) volatile&>
    : unwrap_base<signature<ReturnType(Args...)>, qualifier<false, true, false>> { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...) const volatile&>
    : unwrap_base<signature<ReturnType(Args...)>, qualifier<true, true, false>> { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...)&&>
    : unwrap_base<signature<ReturnType(Args...)>, qualifier<false, false, true>> { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...) const&&>
    : unwrap_base<signature<ReturnType(Args...)>, qualifier<true, false, true>> { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...) volatile&&>
    : unwrap_base<signature<ReturnType(Args...)>, qualifier<false, true, true>> { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...) const volatile&&>
    : unwrap_base<signature<ReturnType(Args...)>, qualifier<true, true, true>> { };

template<typename T>
struct is_function_pointer
    : std::false_type { };

template<typename ReturnType, typename... Args>
struct is_function_pointer<ReturnType(*)(Args...)>
    : std::true_type { };

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
using default_chance = std::integral_constant<std::size_t,
    2UL
>;

// Interfaces for non copyable wrapper:
template<typename /*Signature*/, typename /*Qualifier*/>
struct call_wrapper_operator_interface;

#define FU2_MACRO_DEFINE_CALL_OPERATOR(IS_CONST, IS_VOLATILE, IS_RVALUE) \
    template<typename ReturnType, typename... Args> \
    struct call_wrapper_operator_interface<signature<ReturnType(Args...)>, qualifier<IS_CONST, IS_VOLATILE, IS_RVALUE>> \
    { \
        virtual ~call_wrapper_operator_interface() { } \
      \
        virtual ReturnType operator() (Args&&...) FU2_MACRO_FULL_QUALIFIER(IS_CONST, IS_VOLATILE, IS_RVALUE) = 0; \
      \
    };

FU2_MACRO_EXPAND_3(FU2_MACRO_DEFINE_CALL_OPERATOR)

#undef FU2_MACRO_DEFINE_CALL_OPERATOR

template<typename /*Signature*/, typename /*Qualifier*/, bool /*Copyable*/>
struct call_wrapper_interface;

template<typename ReturnType, typename... Args, typename Qualifier>
struct call_wrapper_interface<signature<ReturnType(Args...)>, Qualifier, false>
     : call_wrapper_operator_interface<signature<ReturnType(Args...)>, Qualifier>
{
    virtual ~call_wrapper_interface() { }

    /// Returns true if the implementation can be allocated in-place in the given region.
    virtual bool can_allocate_inplace(std::size_t size) const = 0;

    /// Placed move
    virtual void move_unique_inplace(call_wrapper_interface* ptr) = 0;

    /// Move the implementation to the heap
    virtual call_wrapper_interface* move_to_heap() = 0;

}; // struct call_wrapper_interface

/// Interface: copyable wrapper
template<typename ReturnType, typename... Args, typename Qualifier>
struct call_wrapper_interface<signature<ReturnType(Args...)>, Qualifier, true>
     : call_wrapper_interface<signature<ReturnType(Args...)>, Qualifier, false>
{
    virtual ~call_wrapper_interface() { }

    /// Placed move
    virtual void move_inplace(call_wrapper_interface* ptr) = 0;

    /// Move the implementation to the heap
    virtual call_wrapper_interface* move_to_heap() = 0;

    /// Placed clone
    virtual void clone_inplace(call_wrapper_interface<signature<ReturnType(Args...)>, Qualifier, false>* ptr) const = 0;

    /// Allocated clone
    virtual call_wrapper_interface* clone_heap() const = 0;

}; // struct call_wrapper_interface

template<typename /*T*/, typename /*Signature*/, typename /*Qualifier*/, bool /*Copyable*/, std::size_t /*Chance*/ = default_chance::value>
class call_wrapper_implementation;

template <typename /*T*/, typename /*Signature*/, typename /*Qualifier*/, bool /*Copyable*/>
struct call_wrapper_operator_implementation;

#define FU2_MACRO_DEFINE_CALL_OPERATOR(IS_CONST, IS_VOLATILE, IS_RVALUE) \
    template<typename T, typename ReturnType, typename... Args, bool Copyable> \
    struct call_wrapper_operator_implementation<T, signature<ReturnType(Args...)>, qualifier<IS_CONST, IS_VOLATILE, IS_RVALUE>, Copyable> \
         : call_wrapper_interface<signature<ReturnType(Args...)>, qualifier<IS_CONST, IS_VOLATILE, IS_RVALUE>, Copyable> \
    { \
        virtual ~call_wrapper_operator_implementation() { } \
      \
        ReturnType operator() (Args&&... args) FU2_MACRO_FULL_QUALIFIER(IS_CONST, IS_VOLATILE, IS_RVALUE) final override \
        { \
            using base = call_wrapper_implementation<T, signature<ReturnType(Args...)>, qualifier<IS_CONST, IS_VOLATILE, IS_RVALUE>, Copyable>; \
          \
            return FU2_MACRO_MOVE_IF(IS_RVALUE)(static_cast<base FU2_MACRO_NO_REF_QUALIFIER(IS_CONST, IS_VOLATILE) *>(this)->_impl)(std::forward<Args>(args)...); \
        } \
    };

FU2_MACRO_EXPAND_3(FU2_MACRO_DEFINE_CALL_OPERATOR)

#undef FU2_MACRO_DEFINE_CALL_OPERATOR

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
    static bool can_allocate_inplace(std::size_t)
    {
        return false;
    }
};

template<typename T, typename Signature, typename Qualifier, bool Copyable, std::size_t Chance>
using generate_next_impl_t = call_wrapper_implementation<T, Signature, Qualifier, Copyable, (Chance > 0) ? Chance - 1 : 0>;

template<typename T, typename Signature, typename Qualifier, std::size_t Chance>
class call_wrapper_implementation<T, Signature, Qualifier, false, Chance>
    : public call_wrapper_operator_implementation<T, Signature, Qualifier, false>
{
    using interface = call_wrapper_interface<Signature, Qualifier, false>;

    using next_implementation = generate_next_impl_t<T, Signature, Qualifier, false, Chance>;

public:
    T _impl;

    template<typename I>
    call_wrapper_implementation(I&& impl)
        : _impl(std::forward<I>(impl)) { }

    call_wrapper_implementation() = delete;
    call_wrapper_implementation(call_wrapper_implementation const&) = delete;
    call_wrapper_implementation(call_wrapper_implementation&&) = delete;
    call_wrapper_implementation& operator= (call_wrapper_implementation const&) = delete;
    call_wrapper_implementation& operator= (call_wrapper_implementation&&) = delete;

    virtual ~call_wrapper_implementation() { }

    /// Returns true if the implementation can be allocated in-place in the given region.
    bool can_allocate_inplace(std::size_t size) const final override
    {
        return can_allocate_inplace_helper<
            next_implementation, Chance
        >::can_allocate_inplace(size);
    }

    /// Placed move
    void move_unique_inplace(interface* ptr) final override
    {
        new (ptr) next_implementation(std::move(_impl));
    }

    /// Move the implementation to the heap
    interface* move_to_heap() final override
    {
        return new generate_next_impl_t<T, Signature, Qualifier, false, 0UL>(std::move(_impl));
    }

    using call_wrapper_operator_implementation<T, Signature, Qualifier, false>::operator();

}; // struct call_wrapper_implementation

/// Interface: copyable wrapper
template<typename T, typename Signature, typename Qualifier, std::size_t Chance>
class call_wrapper_implementation<T, Signature, Qualifier, true, Chance>
    : public call_wrapper_operator_implementation<T, Signature, Qualifier, true>
{
    using interface = call_wrapper_interface<Signature, Qualifier, true>;

    using noncopyable_interface = call_wrapper_interface<Signature, Qualifier, false>;

    using next_implementation = generate_next_impl_t<T, Signature, Qualifier, true, Chance>;

public:
    T _impl;

    template<typename I>
    call_wrapper_implementation(I&& impl)
        : _impl(std::forward<I>(impl)) { }

    call_wrapper_implementation() = delete;
    call_wrapper_implementation(call_wrapper_implementation const&) = delete;
    call_wrapper_implementation(call_wrapper_implementation&&) = delete;
    call_wrapper_implementation& operator= (call_wrapper_implementation const&) = delete;
    call_wrapper_implementation& operator= (call_wrapper_implementation&&) = delete;

    virtual ~call_wrapper_implementation() { }

    /// Returns true if the implementation can be allocated in-place in the given region.
    bool can_allocate_inplace(std::size_t size) const final override
    {
        return can_allocate_inplace_helper<
            next_implementation, Chance
        >::can_allocate_inplace(size);
    }

    /// Placed move
    void move_unique_inplace(noncopyable_interface* ptr) final override
    {
        new (ptr) next_implementation(std::move(_impl));
    }

    /// Placed move
    void move_inplace(interface* ptr) final override
    {
        new (ptr) next_implementation(std::move(_impl));
    }

    /// Move the implementation to the heap
    interface* move_to_heap() final override
    {
        return new generate_next_impl_t<T, Signature, Qualifier, true, 0UL>(std::move(_impl));
    }

    /// Placed clone
    void clone_inplace(noncopyable_interface* ptr) const final override
    {
        new (ptr) next_implementation(_impl);
    }

    /// Allocated clone
    interface* clone_heap() const final override
    {
        return new next_implementation(_impl);
    }

    using call_wrapper_operator_implementation<T, Signature, Qualifier, true>::operator();

}; // struct call_wrapper_implementation

template<typename T, std::size_t Capacity>
struct storage_base_t
{
    storage_base_t() { }

    storage_base_t(T* impl)
        : _impl(impl) { }

    T* _impl;

    std::uint8_t _locale[Capacity];

    inline bool is_inplace() const
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

    inline bool is_inplace() const
    {
        return false;
    }

    inline void weak_deallocate()
    {
        if (_impl)
            delete _impl;
    }

}; // struct storage_base_t

template<typename /*Signature*/, typename /*Qualifier*/, typename /*Config*/>
struct storage_t;

template<typename ReturnType, typename... Args, typename Qualifier, typename Config>
struct storage_t<signature<ReturnType(Args...)>, Qualifier, Config>
    : public storage_base_t<call_wrapper_interface<signature<ReturnType(Args...)>, Qualifier, Config::is_copyable>, Config::capacity>
{
    // Call wrapper interface
    using interface_t = call_wrapper_interface<
        signature<ReturnType(Args...)>, Qualifier, Config::is_copyable
    >;

    // Call wrapper implementation
    template<typename T>
    using implementation_t = call_wrapper_implementation<
        T, signature<ReturnType(Args...)>, Qualifier, Config::is_copyable
    >;

    // Storage base type
    using base_t = storage_base_t<
        interface_t, Config::capacity
    >;

    template<typename T>
    using is_local_allocateable =
        std::integral_constant<bool,
            required_capacity_to_allocate_inplace<T>::value <= Config::capacity>;

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

    inline bool is_allocated() const
    {
        return this->_impl != nullptr;
    }

    inline void change_to_locale()
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
    inline void copy_assign(T const& right)
    {
        this->weak_deallocate();
        weak_copy_assign(right);
    }

    template<typename T>
    inline void do_copy_allocate(T const& right)
    {
        change_to_locale();
        right._impl->clone_inplace(this->_impl);
    }

    // Copy assign with in-place capability.
    template<typename RightConfig,
             typename std::enable_if<(Config::capacity > 0UL) && RightConfig::is_copyable>::type* = nullptr>
    void weak_copy_assign(storage_t<signature<ReturnType(Args...)>, Qualifier, RightConfig> const& right)
    {
        if (!right.is_allocated())
            clean(); // Deallocate if right is unallocated
        else if (right._impl->can_allocate_inplace(Config::capacity))
            do_copy_allocate(right);
        else
            this->_impl = right._impl->clone_heap(); // heap clone
    }

    // Copy assign with no in-place capability.
    template<typename RightConfig,
             typename std::enable_if<(Config::capacity == 0UL) && RightConfig::is_copyable>::type* = nullptr>
    void weak_copy_assign(storage_t<signature<ReturnType(Args...)>, Qualifier, RightConfig> const& right)
    {
        if (!right.is_allocated())
            clean(); // Deallocate if right is unallocated
        else
            this->_impl = right._impl->clone_heap(); // heap clone
    }

    template<typename T>
    static bool can_allocate_inplace(T const& right)
    {
        return right._impl->can_allocate_inplace(Config::capacity);
    }

    template<typename T>
    void do_move_allocate_inplace(tag<true>, T&& right)
    {
        change_to_locale();
        right._impl->move_inplace(this->_impl);
        right.deallocate();
    }

    template<typename T>
    void do_move_allocate_inplace(tag<false>, T&& right)
    {
        change_to_locale();
        right._impl->move_unique_inplace(this->_impl);
        right.deallocate();
    }

    template<typename T>
    inline void do_move_allocate_to_heap(T&& right)
    {
        this->_impl = right._impl->move_to_heap();
        right.deallocate();
    }

    template<typename T>
    inline void move_assign(T&& right)
    {
        this->weak_deallocate();
        weak_move_assign(std::forward<T>(right));
    }

    template<typename RightConfig,
             typename std::enable_if<(Config::capacity > 0UL) && deduce_t<RightConfig>::value>::type* = nullptr>
    void weak_move_assign(storage_t<signature<ReturnType(Args...)>, Qualifier, RightConfig>&& right)
    {
        if (!right.is_allocated())
            clean(); // Deallocate if right is unallocated
        else if (can_allocate_inplace(right))
            do_move_allocate_inplace(tag<Config::is_copyable>{}, std::move(right));
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

    template<typename RightConfig,
             typename std::enable_if<(Config::capacity == 0UL) && deduce_t<RightConfig>::value>::type* = nullptr>
    void weak_move_assign(storage_t<signature<ReturnType(Args...)>, Qualifier, RightConfig>&& right)
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

template<typename /*Signature*/, typename /*Qualifier*/, typename /*Config*/>
class function;

template <typename /*Fn*/>
struct call_operator;

#define FU2_MACRO_DEFINE_CALL_OPERATOR(IS_CONST, IS_VOLATILE, IS_RVALUE) \
    template<typename ReturnType, typename... Args, typename Config> \
    struct call_operator<function<signature<ReturnType(Args...)>, qualifier<IS_CONST, IS_VOLATILE, IS_RVALUE>, Config>> \
    { \
        ReturnType operator()(Args... args) FU2_MACRO_FULL_QUALIFIER(IS_CONST, IS_VOLATILE, IS_RVALUE) \
        { \
            using base = function<signature<ReturnType(Args...)>, qualifier<IS_CONST, IS_VOLATILE, IS_RVALUE>, Config>; \
          \
            return FU2_MACRO_MOVE_IF(IS_RVALUE)(*static_cast<base FU2_MACRO_NO_REF_QUALIFIER(IS_CONST, IS_VOLATILE) *>(this)->_storage._impl)(std::forward<Args>(args)...); \
        } \
    };

FU2_MACRO_EXPAND_3(FU2_MACRO_DEFINE_CALL_OPERATOR)

#undef FU2_MACRO_DEFINE_CALL_OPERATOR

template<typename ReturnType, typename... Args, typename Qualifier, typename Config>
class function<signature<ReturnType(Args...)>, Qualifier, Config>
    : public call_operator<function<signature<ReturnType(Args...)>, Qualifier, Config>>,
      public signature<ReturnType(Args...)>,
      public copyable<Config::is_copyable>
{
    template<typename, typename, typename>
    friend class function;

    friend struct call_operator<function>;

    // Is a true type if the given function is copyable correct to this.
    template<bool RightCopyable>
    using is_copyable_correct_to_this = is_copyable_correct<
        Config::is_copyable, RightCopyable
    >;

    // Is a true type if the given function pointer is assignable to this.
    template<typename T>
    using is_function_pointer_assignable_to_this = std::integral_constant<bool,
        is_function_pointer<T>::value &&
        !Qualifier::is_volatile
    >;

    // Is a true type if the functor class is assignable to this.
    template<typename T>
    using is_functor_assignable_to_this = std::integral_constant<std::size_t,
        !is_function_pointer_assignable_to_this<T>::value &&
        is_callable_with_qualifiers<T, ReturnType(Args...), Qualifier>::value
    >;

    // Implementation storage
    storage_t<signature<ReturnType(Args...)>, Qualifier, Config> _storage;

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
    template<typename RightConfig, typename std::enable_if<RightConfig::is_copyable>::type* = nullptr>
    explicit function(function<signature<ReturnType(Args...)>, Qualifier, RightConfig> const& right)
    {
        _storage.weak_copy_assign(right._storage);
    }

    /// Move construct
    template<typename RightConfig,
             typename = typename std::enable_if<is_copyable_correct_to_this<RightConfig::is_copyable>::value>::type>
    explicit function(function<signature<ReturnType(Args...)>, Qualifier, RightConfig>&& right)
    {
        _storage.weak_move_assign(std::move(right._storage));
    }

    /// Constructor taking a function pointer
    template<typename T,
             typename = typename std::enable_if<is_function_pointer_assignable_to_this<T>::value>::type, typename = void>
    function(T function_pointer)
        : function(functor_box_of<T>(std::forward<T>(function_pointer))) { }

    /// Constructor taking a functor
    template<typename T, typename = typename std::enable_if<is_functor_assignable_to_this<T>::value>::type>
    function(T functor)
    {
        _storage.weak_allocate(std::forward<T>(functor));
    }

    explicit function(std::nullptr_t)
        : _storage() { }

    /// Copy assign
    template<typename RightConfig, typename std::enable_if<RightConfig::is_copyable>::type* = nullptr>
    function& operator= (function<signature<ReturnType(Args...)>, Qualifier, RightConfig> const& right)
    {
        _storage.copy_assign(right._storage);
        return *this;
    }

    /// Move assign
    template<typename RightConfig,
             typename std::enable_if<is_copyable_correct_to_this<RightConfig::is_copyable>::value>::type* = nullptr>
    function& operator= (function<signature<ReturnType(Args...)>, Qualifier, RightConfig>&& right)
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

    using call_operator<function>::operator();

}; // class function

// Default capacity for small functor optimization
using default_capacity = std::integral_constant<std::size_t,
    32UL
>;

#undef FU2_MACRO_IF
#undef FU2_MACRO_IF_true
#undef FU2_MACRO_IF_false
#undef FU2_MACRO_MOVE_IF
#undef FU2_MACRO_MOVE_IF_true
#undef FU2_MACRO_MOVE_IF_false
#undef FU2_MACRO_NO_REF_QUALIFIER
#undef FU2_MACRO_FULL_QUALIFIER
#undef FU2_MACRO_EXPAND_3

} // inline namespace v1

} // namespace detail

/// Function wrapper base
template<typename Signature, std::size_t Capacity, bool Copyable>
using function_base = detail::function<
    typename detail::unwrap<Signature>::signature,
    typename detail::unwrap<Signature>::qualifier,
    detail::config<Capacity, Copyable>
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

} // namespace fu2

#endif // fu2_included_function_hpp__
