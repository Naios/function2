
//  Copyright 2015-2017 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#ifndef FU2_INCLUDED_FUNCTION2_HPP__
#define FU2_INCLUDED_FUNCTION2_HPP__

#include <cassert>
#include <cstdlib>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

// Defines:
// - FU2_MACRO_DISABLE_EXCEPTIONS
#if defined(_MSC_VER)
#if !defined(_HAS_EXCEPTIONS) || (_HAS_EXCEPTIONS == 0)
#define FU2_MACRO_DISABLE_EXCEPTIONS
#endif
#elif defined(__clang__)
#if !(__EXCEPTIONS && __has_feature(cxx_exceptions))
#define FU2_MACRO_DISABLE_EXCEPTIONS
#endif
#elif defined(__GNUC__)
#if !__EXCEPTIONS
#define FU2_MACRO_DISABLE_EXCEPTIONS
#endif
#endif

#if !defined(FU2_NO_FUNCTIONAL_HEADER) || !defined(FU2_MACRO_DISABLE_EXCEPTIONS)
#include <functional>
#endif

#if !defined(FU2_MACRO_DISABLE_EXCEPTIONS)
#include <exception>
#endif

namespace fu2 {
inline namespace v5 {
namespace detail {
// Equivalent to C++17's std::void_t which is targets a bug in GCC,
// that prevents correct SFINAE behavior.
// See http://stackoverflow.com/questions/35753920 for details.
template <typename...>
struct deduce_to_void : std::common_type<void> {};

template <typename... T>
using always_void_t = typename deduce_to_void<T...>::type;

/// Configuration trait to configure the function_base class.
template <bool Owning, bool Copyable, std::size_t Capacity /*,
          bool PartialApplyable*/>
struct config {
  // Is true if the function is copyable.
  static constexpr auto const is_owning = Owning;

  // Is true if the function is copyable.
  static constexpr auto const is_copyable = Copyable;

  // The internal capacity of the function
  // used in small functor optimization.
  static constexpr auto const capacity = Capacity;

  // Is true when the function is assignable with less arguments.
  // static constexpr auto const is_partial_applyable = PartialApplyable;
};

/// A config which isn't compatible to other configs
template <bool Throws, bool HasStrongExceptGuarantee, typename... Args>
struct property {
  // Is true when the function throws an exception on empty invocation.
  static constexpr auto const is_throwing = Throws;

  // Is true when the function throws an exception on empty invocation.
  static constexpr auto const is_strong_exception_guaranteed = Throws;
};

/// Provides utilities for invocing callable objects
namespace invocation {
/// Invokes the given callable object with the given arguments
template <typename Callable, typename... Args>
constexpr auto invoke(Callable&& callable, Args&&... args) noexcept(
    noexcept(std::forward<Callable>(callable)(std::forward<Args>(args)...)))
    -> decltype(std::forward<Callable>(callable)(std::forward<Args>(args)...)) {

  return std::forward<Callable>(callable)(std::forward<Args>(args)...);
}
/// Invokes the given member function pointer
template <typename T, typename Type, typename Self, typename... Args>
constexpr auto invoke(Type T::*member, Self&& self, Args&&... args) noexcept(
    noexcept((std::forward<Self>(self).*member)(std::forward<Args>(args))))
    -> decltype((std::forward<Self>(self).*member)(std::forward<Args>(args))) {
  return (std::forward<Self>(self).*member)(std::forward<Args>(args));
}
} // end namespace invocation

/// Declares the namespace which provides the functionality to work with a
/// type-erased object.
namespace type_erasure {
/// Store the allocator inside the box
template <typename T, typename Allocator>
struct box : public Allocator {
  T value_;

  explicit box(T value, Allocator allocator)
      : Allocator(std::move(allocator)), value_(std::move(value)) {
  }

  /// Allocates space through the boxed allocator
  box* box_allocate() const {
    // TODO find a solution for this circular issue
    using RealAllocator = typename std::allocator_traits<
        std::decay_t<Allocator>>::template rebind_alloc<box<T, Allocator>>;
    RealAllocator allocator(*static_cast<Allocator const*>(this));

    return static_cast<box*>(allocator.allocate(1U));
  }

  /// Destroys the box through the given allocator
  static void box_deallocate(box* me) {
    using RealAllocator = typename std::allocator_traits<
        std::decay_t<Allocator>>::template rebind_alloc<box<T, Allocator>>;
    RealAllocator allocator(*static_cast<Allocator const*>(me));

    me->~box();
    allocator.deallocate(me, 1U);
  }
};

/// Creates a box containing the given value and allocator
template <typename T, typename Allocator = std::allocator<std::decay_t<T>>>
auto make_box(T&& value, Allocator&& allocator = Allocator{}) {
  return box<std::decay_t<T>, std::decay_t<Allocator>>{
      std::forward<T>(value), std::forward<Allocator>(allocator)};
}

template <typename T>
struct is_box : std::false_type {};
template <typename T, typename Allocator>
struct is_box<box<T, Allocator>> : std::true_type {};

/// Provides access to the pointer to a heal allocated erased object
/// as well to the inplace storage.
typedef union {
  /// The pointer we use if the object is on the heap
  void* ptr_;
  /// The first field of the inplace storage
  std::size_t inplace_storage_;
} data_accessor;

/// See opcode::op_fetch_empty
constexpr void write_empty(data_accessor* accessor, bool empty) noexcept {
  accessor->inplace_storage_ = std::size_t(empty);
}

template <typename From, typename To>
using transfer_const_t =
    std::conditional_t<std::is_const<std::remove_pointer_t<From>>::value,
                       std::add_const_t<To>, To>;
template <typename From, typename To>
using transfer_volatile_t =
    std::conditional_t<std::is_volatile<std::remove_pointer_t<From>>::value,
                       std::add_volatile_t<To>, To>;

/// The retriever when the object is allocated inplace
template <typename T, typename Accessor>
constexpr auto retrieve(std::true_type /*is_inplace*/, Accessor from,
                        std::size_t from_capacity)
    -> transfer_const_t<Accessor, transfer_volatile_t<Accessor, void*>>

{
  /// Process the command by using the data on the internal capacity
  auto storage = &(from->inplace_storage_);
  auto inplace = const_cast<void*>(static_cast<void const*>(storage));
  return std::align(alignof(T), sizeof(T), inplace, from_capacity);
}

/// The retriever which is used when the object is allocated
/// through the allocator
template <typename T, typename Accessor>
constexpr auto retrieve(std::false_type /*is_inplace*/, Accessor from,
                        std::size_t /*from_capacity*/) {

  return from->ptr_;
}

/// For allowing private access to erasure
struct erasure_attorney {
  /// Invoke the function of the erasure at the given index
  ///
  /// We define this out of class to be able to forward the qualified
  /// erasure correctly.
  template <std::size_t Index, typename Erasure, typename... Args>
  static constexpr auto invoke(Erasure&& erasure, Args&&... args) noexcept(
      noexcept(std::forward<Erasure>(erasure).vtable_.template invoke<Index>(
          erasure.opaque_ptr(), erasure.capacity(),
          std::forward<Args>(args)...)))
      -> decltype(erasure.vtable_.template invoke<Index>(
          erasure.opaque_ptr(), erasure.capacity(),
          std::forward<Args>(args)...)) {
    // Add data pointer and the capacity to the arguments
    return erasure.vtable_.template invoke<Index>(
        erasure.opaque_ptr(), erasure.capacity(), std::forward<Args>(args)...);
  }
};

namespace invocation_table {
#if defined(FU2_NO_FUNCTIONAL_HEADER) && !defined(FU2_MACRO_DISABLE_EXCEPTIONS)
struct bad_function_call : std::exception {
  bad_function_call() {
  }

  char const* what() const throw() override {
    return "bad function call";
  }
};
#else
using std::bad_function_call;
#endif

#define FU2_EXPAND_QUALIFIERS(F)                                               \
  F(, , )                                                                      \
  F(const, , )                                                                 \
  F(, volatile, )                                                              \
  F(const, volatile, )                                                         \
  F(, , &)                                                                     \
  F(const, , &)                                                                \
  F(, volatile, &)                                                             \
  F(const, volatile, &)                                                        \
  F(, , &&)                                                                    \
  F(const, , &&)                                                               \
  F(, volatile, &&)                                                            \
  F(const, volatile, &&)

/// Calls std::abort on empty function calls
[[noreturn]] inline void throw_or_abort(std::false_type /*is_throwing*/) {
  std::abort();
}
/// Throws bad_function_call on empty funciton calls
[[noreturn]] inline void throw_or_abort(std::true_type /*is_throwing*/) {
#ifdef FU2_MACRO_DISABLE_EXCEPTIONS
  throw_or_abort(std::false_type{});
#else
  throw bad_function_call{};
#endif
}

template <typename T>
struct function_trait;

#define FU2_DEFINE_FUNCTION_TRAIT(CONST, VOLATILE, REFERENCE)                  \
  template <typename Ret, typename... Args>                                    \
  struct function_trait<Ret(Args...) CONST VOLATILE REFERENCE> {               \
    using pointer_type = Ret (*CONST VOLATILE REFERENCE)(                      \
        data_accessor CONST VOLATILE*, std::size_t capacity, Args...);         \
    template <typename T, bool IsInplace>                                      \
    struct internal_invoker {                                                  \
      static Ret invoke(data_accessor CONST VOLATILE* data,                    \
                        std::size_t capacity, Args... args) {                  \
        auto obj = retrieve<T>(std::integral_constant<bool, IsInplace>{},      \
                               data, capacity);                                \
        auto box = static_cast<T CONST VOLATILE*>(obj);                        \
        return invocation::invoke(                                             \
            static_cast<decltype(box->value_) REFERENCE>(box->value_),         \
            std::move(args)...);                                               \
      }                                                                        \
    };                                                                         \
                                                                               \
    template <typename T>                                                      \
    struct accept_sfinae : std::enable_if<std::is_convertible<                 \
                               decltype(invocation::invoke(                    \
                                   std::declval<T CONST VOLATILE REFERENCE>(), \
                                   std::declval<Args>()...)),                  \
                               Ret>::value> {};                                \
                                                                               \
    template <bool Throws>                                                     \
    struct empty_invoker {                                                     \
      static Ret invoke(data_accessor CONST VOLATILE* /*data*/,                \
                        std::size_t /*capacity*/, Args... /*args*/) {          \
        throw_or_abort(std::integral_constant<bool, Throws>{});                \
      }                                                                        \
    };                                                                         \
  };

FU2_EXPAND_QUALIFIERS(FU2_DEFINE_FUNCTION_TRAIT)
#undef FU2_DEFINE_FUNCTION_TRAIT

/// Deduces to the function pointer to the given signature
template <typename Signature>
using function_pointer_of = typename function_trait<Signature>::pointer_type;

/// A packed tuple of signatures
template <typename... Args>
using invoke_table_of_t = std::tuple<function_pointer_of<Args>...>;

/// SFINAES out if the object T isn't invocable with the given signature
template <typename T, typename Signature>
using accept_sfinae_t =
    typename function_trait<Signature>::template accept_sfinae<T>::type;

template <std::size_t Index, typename Function, typename... Signatures>
struct operator_impl;

#define FU2_DEFINE_FUNCTION_TRAIT(CONST, VOLATILE, REFERENCE)                  \
  template <std::size_t Index, typename Function, typename Ret,                \
            typename... Args, typename Next, typename... Signatures>           \
  struct operator_impl<Index, Function, Ret(Args...) CONST VOLATILE REFERENCE, \
                       Next, Signatures...>                                    \
      : operator_impl<Index + 1, Function, Next, Signatures...> {              \
                                                                               \
    using operator_impl<Index + 1, Function, Next, Signatures...>::operator(); \
                                                                               \
    Ret operator()(Args... args) CONST VOLATILE REFERENCE {                    \
      auto function = static_cast<Function CONST VOLATILE*>(this);             \
      return erasure_attorney::invoke<Index>(                                  \
          static_cast<decltype(function->erasure_) REFERENCE>(                 \
              function->erasure_),                                             \
          std::move(args)...);                                                 \
    }                                                                          \
  };                                                                           \
  template <std::size_t Index, typename Function, typename Ret,                \
            typename... Args>                                                  \
  struct operator_impl<Index, Function,                                        \
                       Ret(Args...) CONST VOLATILE REFERENCE> {                \
                                                                               \
    Ret operator()(Args... args) CONST VOLATILE REFERENCE {                    \
      auto function = static_cast<Function CONST VOLATILE*>(this);             \
      return erasure_attorney::invoke<Index>(                                  \
          static_cast<decltype(function->erasure_) REFERENCE>(                 \
              function->erasure_),                                             \
          std::move(args)...);                                                 \
    }                                                                          \
  };

FU2_EXPAND_QUALIFIERS(FU2_DEFINE_FUNCTION_TRAIT)
#undef FU2_DEFINE_FUNCTION_TRAIT
#undef FU2_EXPAND_QUALIFIERS
} // namespace invocation_table

namespace tables {
/// Identifies the action which is dispatched on the erased object
enum class opcode {
  op_move,         //< Move the object and set the vtable
  op_copy,         //< Copy the object and set the vtable
  op_destroy,      //< Destroy the object and reset the vtable
  op_weak_destroy, //< Destroy the object without resetting the vtable
  op_fetch_empty,  //< Stores true or false into the to storage
                   //< to indicate emptiness
};

/// Abstraction for a vtable together with a command table
/// TODO Add optimization for a single formal argument
/// TODO Add optimization to merge both tables if the function is size optimized
template <typename Property>
class vtable;
template <bool IsThrowing, bool HasStrongExceptGuarantee,
          typename... FormalArgs>
class vtable<property<IsThrowing, HasStrongExceptGuarantee, FormalArgs...>> {
  using command_function_t = void (*)(vtable* /*this*/, opcode /*op*/,
                                      data_accessor* /*from*/,
                                      std::size_t /*from_capacity*/,
                                      data_accessor* /*to*/,
                                      std::size_t /*to_capacity*/);

  using invoke_table_t = invocation_table::invoke_table_of_t<FormalArgs...>;

  command_function_t cmd_;
  invoke_table_t const* vtable_;

  template <typename T>
  struct trait {
    static_assert(is_box<T>::value,
                  "The trait must be specialized with a box!");

    /// The command table
    template <bool IsInplace>
    constexpr static void
    process_cmd(vtable* to_table, opcode op, data_accessor* from,
                std::size_t from_capacity, data_accessor* to,
                std::size_t to_capacity) {

      switch (op) {
        case opcode::op_move: {
          /// Retrieve the pointer to the object
          auto box = static_cast<T*>(retrieve<T>(
              std::integral_constant<bool, IsInplace>{}, from, from_capacity));
          assert(box && "The object must not be over aligned or null!");

          if (!IsInplace) {
            // Just reassign both pointers if we allocated on the heap
            to->ptr_ = from->ptr_;
#ifndef _NDEBUG
            from->ptr_ = nullptr;
#endif
            to_table->set_allocated<T>();
          }
          // The object isn't allocate on the heap
          else {
            construct(std::move(*box), to_table, to, to_capacity);
          }
          return;
        }
        case opcode::op_copy: {
          auto box = static_cast<T const*>(retrieve<T>(
              std::integral_constant<bool, IsInplace>{}, from, from_capacity));
          assert(box && "The object must not be over aligned or null!");

          // Try to allocate the object inplace
          construct(*box, to_table, to, to_capacity);
          return;
        }
        case opcode::op_destroy:
        case opcode::op_weak_destroy: {

          assert(!to && !to_capacity && "Arg overflow!");
          auto box = static_cast<T*>(retrieve<T>(
              std::integral_constant<bool, IsInplace>{}, from, from_capacity));

          if (IsInplace) {
            box->~T();
          } else {
            T::box_deallocate(box);
          }

          if (op == opcode::op_destroy) {
            to_table->set_empty();
          }
          return;
        }
        case opcode::op_fetch_empty: {
          write_empty(to, false);
          return;
        }
      }

      // TODO Use an unreachable intrinsic
      assert(false && "Unreachable!");
      std::exit(-1);
    }

    static constexpr void
    construct(T box, vtable* to_table, data_accessor* to,
              std::size_t to_capacity) noexcept(HasStrongExceptGuarantee) {
      // Try to allocate the object inplace
      void* storage = retrieve<T>(std::true_type{}, to, to_capacity);
      if (storage) {
        to_table->set_inplace<T>();
      } else {
        // Allocate the object through the allocator
        to->ptr_ = storage = box.box_allocate();
        to_table->set_allocated<T>();
      }
      new (storage) T(std::move(box));
    }
  };

  /// The command table
  constexpr static void empty_cmd(vtable* to_table, opcode op,
                                  data_accessor* /*from*/,
                                  std::size_t /*from_capacity*/,
                                  data_accessor* to,
                                  std::size_t /*to_capacity*/) {

    switch (op) {
      case opcode::op_move:
      case opcode::op_copy: {
        to_table->set_empty();
        break;
      }
      case opcode::op_destroy:
      case opcode::op_weak_destroy: {
        // Do nothing
        break;
      }
      case opcode::op_fetch_empty: {
        write_empty(to, true);
        break;
      }
    }
  }

public:
  constexpr vtable() noexcept = default;

  /// Initialize an object at the given position
  template <typename T>
  static constexpr void init(vtable& table, T&& object, data_accessor* to,
                             std::size_t to_capacity) {

    trait<std::decay_t<T>>::construct(std::forward<T>(object), &table, to,
                                      to_capacity);
  }

  /// Initializes the vtable object
  constexpr void init_empty() noexcept {
    // Initialize the new command function
    set_empty();
  }

  /// Moves the object at the given position
  constexpr void move(vtable& to_table, data_accessor* from,
                      std::size_t from_capacity, data_accessor* to,
                      std::size_t to_capacity) const
      noexcept(HasStrongExceptGuarantee) {
    cmd_(&to_table, opcode::op_move, from, from_capacity, to, to_capacity);
  }

  /// Destroys the object at the given position
  constexpr void copy(vtable& to_table, data_accessor const* from,
                      std::size_t from_capacity, data_accessor* to,
                      std::size_t to_capacity) const {
    cmd_(&to_table, opcode::op_copy, const_cast<data_accessor*>(from),
         from_capacity, to, to_capacity);
  }

  /// Destroys the object at the given position
  constexpr void
  destroy(data_accessor* from,
          std::size_t from_capacity) noexcept(HasStrongExceptGuarantee) {
    cmd_(this, opcode::op_destroy, from, from_capacity, nullptr, 0U);
  }

  /// Destroys the object at the given position without invalidating the
  /// vtable
  constexpr void
  weak_destroy(data_accessor* from,
               std::size_t from_capacity) noexcept(HasStrongExceptGuarantee) {
    cmd_(this, opcode::op_weak_destroy, from, from_capacity, nullptr, 0U);
  }

  /// Returns true when the vtable doesn't hold any erased object
  constexpr bool empty() const noexcept {
    data_accessor data;
    cmd_(nullptr, opcode::op_fetch_empty, nullptr, 0U, &data, 0U);
    return bool(data.inplace_storage_);
  }

  /// Invoke the function at the given index
  template <std::size_t Index, typename... Args>
  constexpr auto invoke(Args&&... args) const noexcept(
      noexcept(std::get<Index> (*vtable_)(std::forward<Args>(args)...)))
      -> decltype(std::get<Index>(*vtable_)(std::forward<Args>(args)...)) {
    return std::get<Index>(*vtable_)(std::forward<Args>(args)...);
  }

private:
  template <typename T>
  class tables {
    template <bool IsInplace>
    struct vtable_type : invoke_table_t {
      constexpr vtable_type() noexcept
          : invoke_table_t(std::make_tuple(
                &invocation_table::function_trait<FormalArgs>::
                    template internal_invoker<T, IsInplace>::invoke...)) {
      }
    };

  public:
    constexpr static vtable_type<true> const inplace_table{};
    constexpr static vtable_type<true> const allocated_table{};
  };

  template <typename T>
  constexpr void set_inplace() noexcept {
    vtable_ = &tables<T>::inplace_table;
    cmd_ = &trait<std::decay_t<T>>::template process_cmd<true>;
  }
  template <typename T>
  constexpr void set_allocated() noexcept {
    vtable_ = &tables<T>::allocated_table;
    cmd_ = &trait<std::decay_t<T>>::template process_cmd<false>;
  }

  struct empty_type : invoke_table_t {
    constexpr empty_type() noexcept
        : invoke_table_t(std::make_tuple(
              &invocation_table::function_trait<
                  FormalArgs>::template empty_invoker<IsThrowing>::invoke...)) {
    }
  };

  constexpr static empty_type const empty_table{};

  constexpr void set_empty() noexcept {
    vtable_ = &empty_table;
    cmd_ = &empty_cmd;
  }
};
} // namespace tables

/// A union which makes the pointer to the heap object share the
/// same space with the internal capacity.
/// The storage type is distinguished by multiple versions of the
/// control and vtable.
template <std::size_t Capacity, typename = void>
class internal_capacity {
  /// We extend the union through a technique similar to the tail object hack
  union {
    /// Tag to access the structure in a type-safe way
    data_accessor accessor_;
    /// The internal capacity we use to allocate in-place
    std::aligned_storage_t<Capacity> capacity_;
  };

public:
  constexpr internal_capacity() = default;

  constexpr data_accessor* opaque_ptr() noexcept {
    return &accessor_;
  }
  constexpr data_accessor const* opaque_ptr() const noexcept {
    return &accessor_;
  }

  static constexpr std::size_t capacity() noexcept {
    return Capacity;
  }
};
template <std::size_t Capacity>
class internal_capacity<Capacity,
                        std::enable_if_t<(Capacity < sizeof(void*))>> {
  // Tag to access the structure in a type-safe way
  data_accessor accessor_;

public:
  constexpr internal_capacity() = default;

  constexpr data_accessor* opaque_ptr() noexcept {
    return &accessor_;
  }
  constexpr data_accessor const* opaque_ptr() const noexcept {
    return &accessor_;
  }

  static constexpr std::size_t capacity() noexcept {
    return sizeof(accessor_.inplace_storage_);
  }
};

/// A copyable owning erasure
template <typename Config, typename Property>
class erasure : internal_capacity<Config::capacity> {
  friend struct erasure_attorney;

  using VTable = tables::vtable<Property>;

  VTable vtable_;

public:
  /// Returns the capacity of this erasure
  static constexpr std::size_t capacity() noexcept {
    return internal_capacity<Config::capacity>::capacity();
  }

  constexpr erasure() noexcept {
    vtable_.init_empty();
  }

  explicit constexpr erasure(std::nullptr_t) noexcept {
    vtable_.init_empty();
  }

  constexpr erasure(erasure&& right) noexcept(
      Property::is_strong_exception_guaranteed) {
    right.vtable_.move(vtable_, right.opaque_ptr(), right.capacity(),
                       this->opaque_ptr(), capacity());
  }

  constexpr erasure(erasure const& right) {
    right.vtable_.copy(vtable_, right.opaque_ptr(), right.capacity(),
                       this->opaque_ptr(), capacity());
  }

  template <typename OtherConfig>
  constexpr erasure(erasure<OtherConfig, Property> right) noexcept(
      Property::is_strong_exception_guaranteed) {
    right.vtable_.move(vtable_, right.opaque_ptr(), right.capacity(),
                       this->opaque_ptr(), capacity());
  }

  template <typename T,
            std::enable_if_t<is_box<std::decay_t<T>>::value>* = nullptr>
  constexpr erasure(T&& object) {
    VTable::init(vtable_, std::forward<T>(object), this->opaque_ptr(),
                 capacity());
  }

  ~erasure() {
    vtable_.weak_destroy(this->opaque_ptr(), capacity());
  }

  constexpr erasure&
  operator=(std::nullptr_t) noexcept(Property::is_strong_exception_guaranteed) {
    vtable_.destroy(this->opaque_ptr(), capacity());
    return *this;
  }

  constexpr erasure& operator=(erasure&& right) noexcept(
      Property::is_strong_exception_guaranteed) {
    vtable_.weak_destroy(this->opaque_ptr(), capacity());
    right.vtable_.move(vtable_, right.opaque_ptr(), right.capacity(),
                       this->opaque_ptr(), capacity());
    return *this;
  }

  constexpr erasure& operator=(erasure const& right) {
    vtable_.weak_destroy(this->opaque_ptr(), capacity());
    right.vtable_.copy(vtable_, right.opaque_ptr(), right.capacity(),
                       this->opaque_ptr(), capacity());
    return *this;
  }

  template <typename OtherConfig>
  constexpr erasure& operator=(erasure<OtherConfig, Property> right) noexcept(
      Property::is_strong_exception_guaranteed) {
    vtable_.weak_destroy(this->opaque_ptr(), capacity());
    right.vtable_.move(vtable_, right.opaque_ptr(), right.capacity(),
                       this->opaque_ptr(), capacity());
    return *this;
  }

  template <typename T,
            std::enable_if_t<is_box<std::decay_t<T>>::value>* = nullptr>
  constexpr erasure& operator=(T&& object) {
    vtable_.weak_destroy(this->opaque_ptr(), capacity());
    vtable_.init(std::forward<T>(object), this->opaque_ptr(), capacity());
    return *this;
  }

  /// Returns true when the erasure doesn't hold any erased object
  constexpr bool empty() const noexcept {
    return vtable_.empty();
  }
};
} // namespace type_erasure

/// SFINAES out if the object T isn't invocable with all signatures
template <typename T, typename... Signature>
using enable_if_callable_t = always_void_t<
    type_erasure::invocation_table::accept_sfinae_t<T, Signature>...>;

/// SFINAES out if the given callable is not copyable correct to the left one.
template <typename LeftConfig, typename RightConfig>
using enable_if_copyable_correct_t =
    std::enable_if_t<!LeftConfig::is_copyable || RightConfig::is_copyable>;

template <typename Config, typename Property>
class function;
template <typename Config, bool IsThrowing, bool HasStrongExceptGuarantee,
          typename... Args>
class function<Config, property<IsThrowing, HasStrongExceptGuarantee, Args...>>
    : public type_erasure::invocation_table::operator_impl<
          0U,
          function<Config,
                   property<IsThrowing, HasStrongExceptGuarantee, Args...>>,
          Args...> {

  template <typename, typename>
  friend class function;

  template <std::size_t, typename, typename...>
  friend struct type_erasure::invocation_table::operator_impl;

  using Property = property<IsThrowing, HasStrongExceptGuarantee, Args...>;

  type_erasure::erasure<Config, Property> erasure_;

public:
  /// Default constructor which constructs the function empty
  function() = default;

  /// Copy construction from another copyable function
  /// Move construction from another function
  template <typename RightConfig,
            enable_if_copyable_correct_t<Config, RightConfig>* = nullptr>
  function(function<RightConfig, Property> const& right)
      : erasure_(right.erasure_) {
  }

  /// Move construction from another function
  template <typename RightConfig,
            enable_if_copyable_correct_t<Config, RightConfig>* = nullptr>
  function(function<RightConfig, Property>&& right)
      : erasure_(std::move(right.erasure_)) {
  }

  /// Construction from a callable object which overloads the `()` operator
  template <typename T, typename Allocator = std::allocator<std::decay_t<T>>,
            enable_if_callable_t<T, Args...>* = nullptr>
  function(T callable, Allocator&& allocator = Allocator{})
      : erasure_(type_erasure::make_box(std::forward<T>(callable),
                                        std::forward<Allocator>(allocator))) {
  }

  /// Empty constructs the function
  explicit function(std::nullptr_t np) : erasure_(np) {
  }

  /// Copy assigning from another copyable function
  template <typename RightConfig,
            enable_if_copyable_correct_t<Config, RightConfig>* = nullptr>
  function& operator=(function<RightConfig, Property> const& right) {
    erasure_ = right.erasure_;
    return *this;
  }

  /// Move assigning from another function
  template <typename RightConfig,
            enable_if_copyable_correct_t<Config, RightConfig>* = nullptr>
  function& operator=(function<RightConfig, Property>&& right) {
    erasure_ = std::move(right.erasure_);
    return *this;
  }

  /// Move assigning from a functional object
  template <typename T, enable_if_callable_t<T, Args...>* = nullptr>
  function& operator=(T&& callable) {
    erasure_ = type_erasure::make_box(std::forward<T>(callable));
    return *this;
  }

  /// Clears the function
  function& operator=(std::nullptr_t np) {
    erasure_ = np;
    return *this;
  }

  /// Returns true when the function is empty
  bool empty() const noexcept {
    return erasure_.empty();
  }

  /// Returns true when the function isn't empty
  explicit operator bool() const noexcept {
    return !empty();
  }

  /// Assigns a new target with an optional allocator
  template <typename T, typename Allocator = std::allocator<std::decay_t<T>>,
            enable_if_callable_t<T, Args...>* = nullptr>
  void assign(T&& callable, Allocator&& allocator = Allocator{}) {
    erasure_ = type_erasure::make_box(std::forward<T>(callable),
                                      std::forward<Allocator>(allocator));
  }

  /// Swaps this function with the given function
  void swap(function& other) noexcept(HasStrongExceptGuarantee) {
    if (&other == this) {
      return;
    }

    function cache = std::move(other);
    other = std::move(*this);
    *this = std::move(cache);
  }

  /// Swaps the left function with the right one
  friend void swap(function& left,
                   function& right) noexcept(HasStrongExceptGuarantee) {
    left.swap(right);
  }

  /// TODO Fix the doc
  /// Calls the stored callable object,
  /// returns the result when the function exists
  /// otherwise it throws a fu2::bad_function_call when exceptions are enabled.
  /// When exceptions are disabled std::abort is called.
  using type_erasure::invocation_table::operator_impl<
      0U, function<Config, Property>, Args...>::operator();
};

template <typename Config, typename Property>
bool operator==(function<Config, Property> const& f, std::nullptr_t) {
  return !bool(f);
}

template <typename Config, typename Property>
bool operator!=(function<Config, Property> const& f, std::nullptr_t) {
  return bool(f);
}

template <typename Config, typename Property>
bool operator==(std::nullptr_t, function<Config, Property> const& f) {
  return !bool(f);
}

template <typename Config, typename Property>
bool operator!=(std::nullptr_t, function<Config, Property> const& f) {
  return bool(f);
}

// Internal size of an empty function object
using empty_size = std::integral_constant<
    std::size_t, sizeof(function<detail::config<true, true, 0UL>,
                                 detail::property<true, false, void() const>>)>;

// Default capacity for small functor optimization
using default_capacity = std::integral_constant<
    std::size_t,
    // Aim to size the function object to 32UL
    (empty_size::value < 32UL) ? (32UL - empty_size::value) : 16UL>;
} // namespace detail
} // namespace v5

/// Adaptable function wrapper base for arbitrary functional types.
template <
    /// TODO
    bool IsOwning,
    /// Defines whether the function is copyable or not
    bool IsCopyable,
    /// Defines the internal capacity of the function
    /// for small functor optimization.
    std::size_t Capacity,
    /// Defines whether the function throws an exception on empty function call,
    /// `std::abort` is called otherwise.
    bool IsThrowing,
    /// TODO
    bool HasStrongExceptGuarantee,
    /// Defines whether the function allows assignments from a
    /// function with less arguments.
    /// Reserved
    bool PartialApplyable,
    /// Defines the signature of the function wrapper
    typename... Signatures>
using function_base = detail::function<
    detail::config<IsOwning, IsCopyable, Capacity>,
    detail::property<IsThrowing, HasStrongExceptGuarantee, Signatures...>>;

/// Copyable function wrapper for arbitrary functional types.
template <typename... Signatures>
using function = function_base<true, true, detail::default_capacity::value,
                               true, false, false, Signatures...>;

/// Non copyable function wrapper for arbitrary functional types.
template <typename... Signatures>
using unique_function =
    function_base<true, false, detail::default_capacity::value, true, false,
                  false, Signatures...>;
} // namespace fu2

#undef FU2_NO_FUNCTIONAL_HEADER
#undef FU2_MACRO_DISABLE_EXCEPTIONS

#endif // FU2_INCLUDED_FUNCTION2_HPP__
