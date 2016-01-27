
//  Copyright 2015-2016 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#ifndef fu2_included_function_hpp_
#define fu2_included_function_hpp_

#include <tuple>
#include <memory>
#include <exception>
#include <type_traits>

namespace fu2
{

namespace detail
{

inline namespace v2
{

// Copy enabler helper class
template<bool /*Copyable*/>
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

// Helper to store function signature.
template<typename /*Signature*/>
struct signature;

template<typename ReturnType, typename... Args>
struct signature<ReturnType(Args...)>
{
  // The return type of the function.
  using return_type = ReturnType;

  // The argument types of the function as pack in std::tuple.
  using argument_type = std::tuple<Args...>;
};

// Helper to store function qualifiers.
template<bool Constant, bool Volatile, bool RValue>
struct qualifier
{
  // Is true if the qualifier has const.
  static constexpr bool is_const = Constant;

  // Is true if the qualifier has volatile.
  static constexpr bool is_volatile = Volatile;

  // Is true if the qualifier has r-value reference.
  static constexpr bool is_rvalue = RValue;
};

// Helper to store the function configuration.
template<bool Copyable, std::size_t Capacity, bool Throws>
struct config
{
  // Is true if the function is copyable.
  static constexpr bool is_copyable = Copyable;

  // The internal capacity of the function
  // used in small functor optimization.
  static constexpr std::size_t capacity = Capacity;

  // Is true if the function throws an exception on empty invocation.
  static constexpr bool is_throwing = Throws;
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
using is_callable_with_qualifiers =
  decltype(impl_is_callable_with_qualifiers<Fn>::template test<
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

// Function unwrap trait
template<typename Signature, typename Qualifier>
struct unwrap_base
{
  // The signature of the function
  using signature = Signature;

  // The qualifier of the function
  using qualifier = Qualifier;
};

// Function unwrap trait
template<typename Fn>
struct unwrap
{
  static_assert(sizeof(Fn) < 0,
    "Incompatible signature given, signature must be in the form of "
    " \"ReturnType(Arg...) Qualifier\".");
};

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...)>
  : unwrap_base<
      signature<ReturnType(Args...)>,
      qualifier<false, false, false>
    > { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...) const>
  : unwrap_base<
      signature<ReturnType(Args...)>,
      qualifier<true, false, false>
    > { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...) volatile>
  : unwrap_base<
      signature<ReturnType(Args...)>,
      qualifier<false, true, false>
    > { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...) const volatile>
  : unwrap_base<
      signature<ReturnType(Args...)>,
      qualifier<true, true, false>
    > { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...)&>
  : unwrap_base<
      signature<ReturnType(Args...)>,
      qualifier<false, false, false>
    > { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...) const&>
  : unwrap_base<
      signature<ReturnType(Args...)>,
      qualifier<true, false, false>
    > { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...) volatile&>
  : unwrap_base<
      signature<ReturnType(Args...)>,
      qualifier<false, true, false>
    > { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...) const volatile&>
  : unwrap_base<
      signature<ReturnType(Args...)>,
      qualifier<true, true, false>
    > { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...)&&>
  : unwrap_base<
      signature<ReturnType(Args...)>,
      qualifier<false, false, true>
    > { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...) const&&>
  : unwrap_base<
      signature<ReturnType(Args...)>,
      qualifier<true, false, true>
    > { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...) volatile&&>
  : unwrap_base<
      signature<ReturnType(Args...)>,
      qualifier<false, true, true>
    > { };

template<typename ReturnType, typename... Args>
struct unwrap<ReturnType(Args...) const volatile&&>
  : unwrap_base<
      signature<ReturnType(Args...)>,
      qualifier<true, true, true>
    > { };

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

// Increases the chances when to fall back from in-place
// to heap allocation for move performance.
using default_chance = std::integral_constant<std::size_t,
  2UL
>;

template<typename Signature, bool Copyable>
struct function_vtable;

template<typename ReturnType, typename... Args, bool Copyable>
struct function_vtable<signature<ReturnType(Args...)>, Copyable>
{
  typedef void(*destruct_t)(void* /*destination*/);
  typedef ReturnType(*invoke_t)(void* /*destination*/, Args&&... /*args*/);
  typedef std::size_t(*required_size_t)();
  typedef void (*move_t)(void* /*from*/, void* /*to*/);

  constexpr function_vtable(destruct_t destruct_, invoke_t invoke_,
    required_size_t required_size_, move_t move_)
    : destruct(destruct_), invoke(invoke_),
      required_size(required_size_), move(move_) { }

  destruct_t const destruct;
  invoke_t const invoke;
  required_size_t const required_size;
  move_t const move;
};

template<typename ReturnType, typename... Args>
struct function_vtable<signature<ReturnType(Args...)>, true>
   : function_vtable<signature<ReturnType(Args...)>, false>
{
  typedef void (*copy_t)(void* /*from*/, void* /*to*/);

  constexpr function_vtable(
      typename function_vtable::destruct_t destruct_,
      typename function_vtable::invoke_t invoke_,
      typename function_vtable::required_size_t required_size_,
      typename function_vtable::move_t move_,
      copy_t copy_)
    : function_vtable<signature<ReturnType(Args...)>, false>
      (destruct_, invoke_, required_size_, move_), copy(copy_) { }

  copy_t const copy;
};

// Performs no operation on the given pointer.
static inline void function_wrapper_noop(void* /*dest*/) { }

// Performs no operation on the given pointer.
static inline void function_wrapper_noop2(void* /*dest*/, void* /*dest*/) { }

// Constructs a type T at the given destination with the given arguments.
template<typename T, typename... Args>
static inline void function_wrapper_construct(void* destination, Args... args)
{
  new (destination) typename std::decay<T>::type(std::forward<Args>(args)...);
}

// Destructs a type T at the given destination.
template<typename T>
static inline void function_wrapper_destruct(void* destination)
{
  static_cast<T*>(destination)->~T();
  (void)destination;
}

// Returns the required size of the type to allocate in-place.
template<typename T>
static inline std::size_t function_wrapper_required_size()
{
  return required_capacity_to_allocate_inplace<T>::value;
}

// Returns a zero size.
static inline std::size_t function_wrapper_zero_size()
{
  return 0UL;
}

// Moves the given type at the target location to another one.
template<typename T>
static inline void function_wrapper_move(void* from, void* to)
{
  function_wrapper_construct<T>(to, std::move(*static_cast<T*>(from)));
}

// Copies the given type at the target location to another one.
template<typename T>
static inline void function_wrapper_copy(void* from, void* to)
{
  function_wrapper_construct<T>(to, *static_cast<T*>(from));
}

template<typename /*T*/, typename /*Signature*/, typename /*Qualifier*/>
struct function_wrapper_invoker;

#define FU2_MACRO_DEFINE_CALL_OPERATOR(IS_CONST, IS_VOLATILE, IS_RVALUE) \
  template<typename T, typename ReturnType, typename... Args> \
  struct function_wrapper_invoker< \
    T, \
    signature<ReturnType(Args...)>, \
    qualifier<IS_CONST, IS_VOLATILE, IS_RVALUE> \
  > \
  { \
    static inline ReturnType invoke(void* target, Args&&... args) \
    { \
      return FU2_MACRO_MOVE_IF(IS_RVALUE)(* static_cast< \
        T FU2_MACRO_NO_REF_QUALIFIER(IS_CONST, IS_VOLATILE) *>( \
          target))(std::forward<Args>(args)...); \
    } \
  };

FU2_MACRO_EXPAND_3(FU2_MACRO_DEFINE_CALL_OPERATOR)

#undef FU2_MACRO_DEFINE_CALL_OPERATOR

struct bad_function_call : std::exception
{
  bad_function_call() { }

  virtual char const* what() const throw()
  {
    return "bad function call";
  }
};

template<typename /*Signature*/, bool /*Throws*/>
struct vtable_creator_of_empty_function;

template<typename ReturnType, typename... Args>
struct vtable_creator_of_empty_function<signature<ReturnType(Args...)>, true>
{
  using common_vtable_t = function_vtable<
    signature<ReturnType(Args...)>,
    true
  >;

  // Throws an empty function call
  static ReturnType invoke(void*, Args&&...)
  {
    throw bad_function_call{};
  }

  static common_vtable_t const* create_vtable()
  {
    static constexpr common_vtable_t const vtable(
      function_wrapper_noop,
      invoke,
      function_wrapper_zero_size,
      function_wrapper_noop2,
      function_wrapper_noop2
    );

    return &vtable;
  }
};

template<typename ReturnType, typename... Args>
struct vtable_creator_of_empty_function<signature<ReturnType(Args...)>, false>
{
  using common_vtable_t = function_vtable<
    signature<ReturnType(Args...)>,
    true
  >;

  // Non-Throwing empty function call
  static inline ReturnType invoke(void*, Args&&...)
  {
    std::abort();
  }

  static common_vtable_t const* create_vtable()
  {
    static constexpr common_vtable_t const vtable(
      function_wrapper_noop,
      invoke,
      function_wrapper_zero_size,
      function_wrapper_noop2,
      function_wrapper_noop2
    );

    return &vtable;
  }
};

template<typename /*T*/, typename /*Signature*/,
         typename /*Qualifier*/, bool /*Copyable*/>
struct vtable_creator_of_type;

template<typename T, typename ReturnType, typename... Args, typename Qualifier>
struct vtable_creator_of_type<T, signature<ReturnType(Args...)>,
                              Qualifier, true>
{
  using common_vtable_t = function_vtable<
    signature<ReturnType(Args...)>,
    true
  >;

  static common_vtable_t const* create_vtable()
  {
    static common_vtable_t const vtable(
      function_wrapper_destruct<T>,
      function_wrapper_invoker<
        T, signature<ReturnType(Args...)>, Qualifier
      >::invoke,
      function_wrapper_required_size<T>,
      function_wrapper_move<T>,
      function_wrapper_copy<T>
    );

    return &vtable;
  }
};

template<typename T, typename ReturnType, typename... Args, typename Qualifier>
struct vtable_creator_of_type<T, signature<ReturnType(Args...)>,
                              Qualifier, false>
{
  using common_vtable_t = function_vtable<
    signature<ReturnType(Args...)>,
    true
  >;

  static common_vtable_t const* create_vtable()
  {
    static common_vtable_t const vtable(
      function_wrapper_destruct<T>,
      function_wrapper_invoker<
        T, signature<ReturnType(Args...)>, Qualifier
      >::invoke,
      function_wrapper_required_size<T>,
      function_wrapper_move<T>,
      nullptr
    );

    return &vtable;
  }
};

template<typename /*Signature*/, typename /*Qualifier*/, typename /*Config*/>
struct storage_t;

template<typename ReturnType, typename... Args,
         typename Qualifier, typename Config>
struct storage_t<signature<ReturnType(Args...)>, Qualifier, Config>
{
  using vtable_ptr_t = function_vtable<
    signature<ReturnType(Args...)>,
    Config::is_copyable
  > const*;

  vtable_ptr_t _vtable;

  void* _impl;

  typename std::conditional<(Config::capacity > 0UL),
    typename std::aligned_storage<Config::capacity>::type,
    std::true_type
  >::type _locale;

  storage_t()
  {
    tidy();
  }

  explicit storage_t(storage_t const& right)
  {
    weak_copy_assign(right);
  }

  explicit storage_t(storage_t&& right)
  {
    weak_move_assign(std::forward<storage_t>(right));
  }

  storage_t& operator= (storage_t const& right)
  {
    weak_deallocate();
    weak_copy_assign(right);
    return *this;
  }

  storage_t& operator= (storage_t&& right)
  {
    weak_deallocate();
    weak_move_assign(std::forward<storage_t>(right));
    return *this;
  }

  ~storage_t()
  {
    weak_deallocate();
  }

  // Private API
  inline void weak_deallocate()
  {
    _vtable->destruct(_impl);

    if (_impl != &_locale)
      std::free(_impl);
  }

  // Private API
  void deallocate()
  {
    weak_deallocate();
    tidy();
  }

  inline void tidy()
  {
    _vtable = vtable_creator_of_empty_function<
      signature<ReturnType(Args...)>, Config::is_throwing
    >::create_vtable();
    _impl = nullptr;
  }

  // Allocate in locale capacity.
  template<typename /*T*/>
  inline void allocate_space(std::true_type)
  {
    _impl = &_locale;
  }

  // Allocate on the heap.
  template<typename T>
  inline void allocate_space(std::false_type)
  {
    _impl = std::malloc(sizeof(T));
  }

  template<typename T>
  void weak_allocate_object(T functor)
  {
    using is_local_allocateable = std::integral_constant<bool,
      required_capacity_to_allocate_inplace<
        typename std::decay<T>::type
      >::value <= Config::capacity
    >;

    _vtable = vtable_creator_of_type<
      typename std::decay<T>::type, signature<ReturnType(Args...)>,
      Qualifier, Config::is_copyable
    >::create_vtable();

    allocate_space<typename std::decay<T>::type>(is_local_allocateable{});
    function_wrapper_construct<
      typename std::decay<T>::type
    >(_impl, std::forward<T>(functor));
  }

  void weak_allocate_function_pointer(ReturnType(*function_pointer)(Args...))
  {
    weak_allocate_object([function_pointer](Args&&... args) -> ReturnType
    {
      return function_pointer(std::forward<Args>(args)...);
    });
  }

  // Private API
  template<typename RightConfig,
           typename std::enable_if<RightConfig::is_copyable>::type* = nullptr>
  void weak_copy_assign(storage_t<signature<ReturnType(Args...)>,
                        Qualifier, RightConfig> const& right)
  {
    _vtable = right._vtable;

    std::size_t const required_size = right._vtable->required_size();
    if (right._impl == &right._locale && (Config::capacity >= required_size))
      _impl = &_locale;
    else
      _impl = std::malloc(required_size);

    right._vtable->copy(right._impl, _impl);
  }

  // Private API
  template<typename RightConfig>
  void weak_move_assign(storage_t<signature<ReturnType(Args...)>,
                        Qualifier, RightConfig>&& right)
  {
    _vtable = right._vtable;

    std::size_t const required_size = right._vtable->required_size();
    if (right._impl == &right._locale)
    {
      if (Config::capacity >= required_size)
        _impl = &_locale;
      else
        _impl = std::malloc(required_size);

      right._vtable->move(right._impl, _impl);
      right.deallocate();
    }
    else
    {
      // Steal the ownership
      _impl = right._impl;
      right.tidy();
    }
  }

  inline bool empty() const { return _impl ? false : true; }

}; // struct storage_t

template<typename /*Signature*/, typename /*Qualifier*/, typename /*Config*/>
class function;

template <typename /*Fn*/>
struct call_operator;

#define FU2_MACRO_DEFINE_CALL_OPERATOR(IS_CONST, IS_VOLATILE, IS_RVALUE) \
  template<typename ReturnType, typename... Args, typename Config> \
  struct call_operator<function<signature<ReturnType(Args...)>, \
                                qualifier<IS_CONST, IS_VOLATILE, IS_RVALUE>, \
                                Config>> \
  { \
    ReturnType operator()(Args... args) \
      FU2_MACRO_FULL_QUALIFIER(IS_CONST, IS_VOLATILE, IS_RVALUE) \
    { \
      using base = function<signature<ReturnType(Args...)>, \
                            qualifier<IS_CONST, IS_VOLATILE, IS_RVALUE>, \
                            Config>; \
      \
      auto const me = static_cast< \
        base FU2_MACRO_NO_REF_QUALIFIER(IS_CONST, IS_VOLATILE) *>(this); \
      \
      return me->_storage._vtable->invoke( \
        me->_storage._impl, std::forward<Args>(args)...); \
    } \
  };

FU2_MACRO_EXPAND_3(FU2_MACRO_DEFINE_CALL_OPERATOR)

#undef FU2_MACRO_DEFINE_CALL_OPERATOR

template<typename ReturnType, typename... Args,
         typename Qualifier, typename Config>
class function<signature<ReturnType(Args...)>, Qualifier, Config>
  : public call_operator<
      function<signature<ReturnType(Args...)>, Qualifier, Config>
    >,
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
    std::is_convertible<T, ReturnType(*)(Args...)>::value &&
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

public:
  function() = default;

  // Copy construct
  template<typename RightConfig,
           typename std::enable_if<RightConfig::is_copyable>::type* = nullptr>
  explicit function(function<signature<ReturnType(Args...)>,
                                       Qualifier, RightConfig> const& right)
  {
    _storage.weak_copy_assign(right._storage);
  }

  // Move construct
  template<typename RightConfig,
           typename = typename std::enable_if<
            is_copyable_correct_to_this<RightConfig::is_copyable>::value
           >::type>
  explicit function(function<signature<ReturnType(Args...)>,
                             Qualifier, RightConfig>&& right)
  {
    _storage.weak_move_assign(std::move(right._storage));
  }

  // Constructor taking a function pointer
  template<typename T,
           typename = typename std::enable_if<
            is_function_pointer_assignable_to_this<T>::value
          >::type,
          typename = void>
  function(T function_pointer)
  {
    _storage.weak_allocate_function_pointer(std::forward<T>(function_pointer));
  }

  // Constructor taking a functor
  template<typename T,
           typename = typename std::enable_if<
            is_functor_assignable_to_this<T>::value
           >::type>
  function(T functor)
  {
    _storage.weak_allocate_object(std::forward<T>(functor));
  }

  explicit function(std::nullptr_t)
    : _storage() { }

  // Copy assign
  template<typename RightConfig,
           typename std::enable_if<RightConfig::is_copyable>::type* = nullptr>
  function& operator= (function<signature<ReturnType(Args...)>,
                                          Qualifier, RightConfig> const& right)
  {
    _storage.weak_deallocate();
    _storage.weak_copy_assign(right._storage);
    return *this;
  }

  // Move assign
  template<typename RightConfig,
           typename std::enable_if<
            is_copyable_correct_to_this<RightConfig::is_copyable>::value
           >::type* = nullptr>
  function& operator= (function<signature<ReturnType(Args...)>,
                                          Qualifier, RightConfig>&& right)
  {
    _storage.weak_deallocate();
    _storage.weak_move_assign(std::move(right._storage));
    return *this;
  }

  // Copy assign taking a function pointer
  template<typename T,
           typename std::enable_if<
            is_function_pointer_assignable_to_this<T>::value
           >::type* = nullptr>
  function& operator= (T function_pointer)
  {
    _storage.weak_deallocate();
    _storage.weak_allocate_function_pointer(std::forward<T>(function_pointer));
    return *this;
  }

  // Copy assign taking a functor
  template<typename T,
           typename std::enable_if<
            is_functor_assignable_to_this<T>::value
           >::type* = nullptr>
  function& operator= (T functor)
  {
    _storage.weak_deallocate();
    _storage.weak_allocate_object(std::forward<T>(functor));
    return *this;
  }

  function& operator= (std::nullptr_t)
  {
    _storage.deallocate();
    return *this;
  }

  bool empty() const { return _storage.empty(); }
  explicit operator bool() const { return !empty(); }

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

} /// inline namespace v2

} /// namespace detail

/// \brief Adaptable function wrapper base for arbitrary functional types.
template<typename Signature,
     bool Copyable,
     std::size_t Capacity = detail::default_capacity::value,
     bool Throwing = true>
using function_base = detail::function<
  typename detail::unwrap<Signature>::signature,
  typename detail::unwrap<Signature>::qualifier,
  detail::config<Copyable, Capacity, Throwing>
>;

/// \brief Copyable function wrapper for arbitrary functional types.
template<typename Signature>
using function = function_base<
  Signature,
  true
>;

/// \brief Non copyable function wrapper for arbitrary functional types.
template<typename Signature>
using unique_function = function_base<
  Signature,
  false
>;

/// \brief Exception type when invoking empty functional wrappers.
///
/// The exception type thrown through empty function calls
/// when the template parameter 'Throwing' is set to true (default).
using detail::bad_function_call;

} /// namespace fu2

#endif // fu2_included_function_hpp_
