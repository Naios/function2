
//  Copyright 2015-2016 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)

#ifndef FU2_INCLUDED_FUNCTION2_HPP__
#define FU2_INCLUDED_FUNCTION2_HPP__

#include <tuple>
#include <cstdlib>
#include <exception>
#include <type_traits>

// Detect disabled exceptions
#if defined(_MSC_VER)
  #if !defined(_HAS_EXCEPTIONS) || (_HAS_EXCEPTIONS == 0)
    #define FU2_MACRO_DISABLE_EXCEPTIONS
  #endif

  #define FU2_MACRO_EXPECT(EXPRESSION, VALUE) (EXPRESSION)
#else
  #ifdef __clang__
    #if !(__EXCEPTIONS && __has_feature(cxx_exceptions))
      #define FU2_MACRO_DISABLE_EXCEPTIONS
    #endif
  #elif defined(__GNUC__)
    #if !__EXCEPTIONS
      #define FU2_MACRO_DISABLE_EXCEPTIONS
    #endif
  #endif

  #define FU2_MACRO_EXPECT(EXPRESSION, VALUE) \
    __builtin_expect(EXPRESSION, VALUE)
#endif

#if !defined(FU2_NO_FUNCTIONAL_HEADER) || !defined(FU2_MACRO_DISABLE_EXCEPTIONS)
  #include <functional>
#endif

// If macro.
#define FU2_MACRO_IF(cond) \
  FU2_MACRO_IF_ ## cond
#define FU2_MACRO_IF_true(EXPRESSION) EXPRESSION
#define FU2_MACRO_IF_false(EXPRESSION)

// If macro to turn the expression into a r-value expression.
#define FU2_MACRO_MOVE_IF(cond) \
  FU2_MACRO_MOVE_IF_ ## cond
#define FU2_MACRO_MOVE_IF_true(EXPRESSION) std::move(EXPRESSION)
#define FU2_MACRO_MOVE_IF_false(EXPRESSION) (EXPRESSION)

// Qualifier without r-value ref.
#define FU2_MACRO_NO_REF_QUALIFIER(IS_CONST, IS_VOLATILE) \
  FU2_MACRO_IF(IS_CONST)(const) \
  FU2_MACRO_IF(IS_VOLATILE)(volatile)

// Full qualifier
#define FU2_MACRO_FULL_QUALIFIER(IS_CONST, IS_VOLATILE, IS_RVALUE) \
  FU2_MACRO_NO_REF_QUALIFIER(IS_CONST, IS_VOLATILE) \
  FU2_MACRO_IF(IS_RVALUE)(&&)

// Expand the given macro with all possible combinations.
#define FU2_MACRO_EXPAND_ALL(EXPRESSION) \
    EXPRESSION(false, false, false) \
    EXPRESSION(false, false, true) \
    EXPRESSION(false, true, false) \
    EXPRESSION(false, true, true) \
    EXPRESSION(true, false, false) \
    EXPRESSION(true, false, true) \
    EXPRESSION(true, true, false) \
    EXPRESSION(true, true, true)

namespace fu2 {
namespace detail {
inline namespace v4 {

template<typename /*Signature*/, typename /*Qualifier*/, typename /*Config*/>
class function;

// Equivalent to C++17's std::void_t which is targets a bug in GCC,
// that prevents correct SFINAE behavior.
// See http://stackoverflow.com/questions/35753920 for details.
template<typename...>
struct deduce_to_void : std::common_type<void> { };

template<typename... T>
using always_void_t = typename deduce_to_void<T...>::type;

// Is convertible trait to fix MSVC crashes and misbehavior
template<typename From, typename To, typename = always_void_t<>>
struct is_convertible : std::false_type { };

template<typename From, typename To>
struct is_convertible<From, To, always_void_t<
  decltype(To(std::declval<From>()))
>> : std::true_type { };

// Copy enabler helper class
template<bool /*Copyable*/>
struct copyable { };

template <>
struct copyable<false> {
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
struct signature<ReturnType(Args...)> {
  // The return type of the function.
  using return_type = ReturnType;

  // The argument types of the function as pack in std::tuple.
  using argument_type = std::tuple<Args...>;
};

// Helper to store function qualifiers.
template<bool Constant, bool Volatile, bool RValue>
struct qualifier {
  // Is true if the qualifier has const.
  static constexpr auto const is_const = Constant;

  // Is true if the qualifier has volatile.
  static constexpr auto const is_volatile = Volatile;

  // Is true if the qualifier has r-value reference.
  static constexpr auto const is_rvalue = RValue;
};

// Helper to store the function configuration.
template<bool Copyable, std::size_t Capacity,
         bool Throws, bool PartialApplyable>
struct config {
  // Is true if the function is copyable.
  static constexpr auto const is_copyable = Copyable;

  // The internal capacity of the function
  // used in small functor optimization.
  static constexpr auto const capacity = Capacity;

  // Is true when the function throws an exception on empty invocation.
  static constexpr auto const is_throwing = Throws;

  // Is true when the function is assignable with less arguments.
  static constexpr auto const is_partial_applyable = PartialApplyable;
};

template<bool Condition, typename T>
using add_pointer_if = typename std::conditional<
  Condition,
  typename std::add_pointer<T>::type,
  T
>::type;

template<bool Condition, typename T>
using add_const_if = typename std::conditional<
  Condition,
  typename std::add_const<T>::type,
  T
>::type;

template<bool Condition, typename T>
using add_volatile_if = typename std::conditional<
  Condition,
  typename std::add_volatile<T>::type,
  T
>::type;

template<bool Condition, typename T>
using add_lvalue_if = typename std::conditional<
  Condition,
  typename std::add_lvalue_reference<T>::type,
  T
>::type;

// Qualifies the type T as given in the Qualifier config
template<typename T, typename Qualifier, bool IsPointer = false>
using make_qualified_type_t =
  add_pointer_if<IsPointer,
  add_lvalue_if<!IsPointer && !Qualifier::is_rvalue,
  add_volatile_if<Qualifier::is_volatile,
  add_const_if<Qualifier::is_const,
    typename std::decay<typename std::remove_pointer<T>::type>::type>>>>;

// Provides a static wrap method which routes the functor through
struct invocation_wrapper_none {
  template<typename T>
  static auto wrap(T&& functor) -> typename std::decay<T>::type {
    return std::forward<T>(functor);
  }
};

// Type which is inheritable to accept a certain invocation
template<typename InvocationWrapper = invocation_wrapper_none>
struct accept_invocation
  : std::common_type<InvocationWrapper> { };


// Decorate this calls of method pointers
template<typename Signature, typename Qualifier>
struct invocation_wrapper_decorate_this_call;

template<typename ReturnType, typename Callee, typename... Args>
struct invocation_wrapper_decorate_this_call<
  ReturnType(Callee, Args...), qualifier<false, false, false>> {
  template<typename T>
  struct decorator {
    typename std::decay<T>::type decorated_;
    ReturnType operator() (Callee callee, Args&&... args) {
      return (callee->*decorated_)(std::forward<Args>(args)...);
    }
  };

  template<typename T>
  static auto wrap(T&& functor) -> decorator<T> {
    return { std::forward<T>(functor) };
  }
};

// 3) Invocation acceptor which accepts (templated) class method pointers
// from a correct qualified this pointer.
/*template<typename T,
         typename Signature, typename Qualifier, typename Config,
         template<typename...> class Accept,
         typename = always_void_t<>>
struct accept_decorated_this_calls { };

template<typename T,
         typename ReturnType, typename Callee, typename... Args,
         typename Qualifier, typename Config,
         template<typename...> class Accept>
struct accept_decorated_this_calls<T, ReturnType(Callee, Args...),
                                   Qualifier, Config,
  Accept, always_void_t<
    typename std::enable_if<is_convertible<
      decltype((std::declval<Callee>()->*std::declval<T>())(std::declval<Args>()...)),
      ReturnType
    >::value>::type
  >>
  : Accept<invocation_wrapper_decorate_this_call<
      ReturnType(Callee, Args...), Qualifier
    >> { };
*/

// 2) Invocation acceptor which accepts (template) functors and function pointers
// Deduces to an invocation_acceptor on success.
//
// You may define your own specializations of this class to accept more
// types which are accepted as functors or functions.
//
// The given user type is wrappable through a static method named wrap
// of a class you pass to the accept_invocation struct.
template<typename T,
         typename Signature, typename Qualifier, typename Config,
         template<typename...> class Accept,
         typename = always_void_t<>>
struct accept_default_call
  /*: accept_decorated_this_calls<T, Signature, Qualifier, Config, Accept>*/ { };

template<typename T,
         typename ReturnType, typename... Args,
         typename Qualifier, typename Config,
         template<typename...> class Accept>
struct accept_default_call<T, ReturnType(Args...), Qualifier, Config,
  Accept, always_void_t<
    typename std::enable_if<is_convertible<
      decltype(std::declval<
        make_qualified_type_t<T, Qualifier>
      >()(std::declval<Args>()...)),
      ReturnType
    >::value>::type>>
  : Accept<> { };

// 1) Always reject function2 classes
// This is required to prevent MSVC from prioritizing template assignment
// operators over the move constructor when copying is disabled,
// which results in test case failure.
template<typename T,
         typename Signature, typename Qualifier, typename Config,
         template<typename...> class Accept>
struct reject_function2
  : accept_default_call<T, Signature, Qualifier, Config, Accept> { };

template<typename FnReturnType, typename... FnArgs,
         typename FnQualifier, typename FnConfig,
         typename ReturnType, typename... Args,
         typename Qualifier, typename Config,
         template<typename...> class Accept>
struct reject_function2<
  function<signature<FnReturnType(FnArgs...)>, FnQualifier, FnConfig>,
  ReturnType(Args...), Qualifier, Config, Accept
> /*reject*/ { };

template<typename T,
         typename Signature, typename Qualifier, typename Config>
struct invocation_acceptor : reject_function2<
  T, Signature, Qualifier, Config, accept_invocation
> { };

// Is a true type if the left type is copyable correct to the right type.
template<bool LeftCopyable, bool RightCopyable>
using is_copyable_correct = std::integral_constant<bool,
  !(LeftCopyable && !RightCopyable)
>;

// Function unwrap trait
template<typename Signature, typename Qualifier>
struct unwrap_base {
  // The signature of the function
  using signature = Signature;

  // The qualifier of the function
  using qualifier = Qualifier;
};

// Function unwrap trait
template<typename Fn>
struct unwrap {
  static_assert(sizeof(Fn) < 0,
    "Incompatible signature given, signature must be in the form of "
    " \"ReturnType(Arg...) Qualifier\".");
};

// Expand all const, volatile and l-value or r-value qualifiers
#define FU2_MACRO_EXPAND_LVALUE_true(IS_CONST, IS_VOLATILE)
#define FU2_MACRO_EXPAND_LVALUE_false(IS_CONST, IS_VOLATILE) \
  template<typename ReturnType, typename... Args> \
  struct unwrap<ReturnType(Args...) \
    FU2_MACRO_NO_REF_QUALIFIER(IS_CONST, IS_VOLATILE) &> \
    : unwrap_base< \
      signature<ReturnType(Args...)>, \
        qualifier<IS_CONST, IS_VOLATILE, false> \
      > { };

#define FU2_MACRO_DEFINE_SIGNATURE_UNWRAP(IS_CONST, IS_VOLATILE, IS_RVALUE) \
  template<typename ReturnType, typename... Args> \
  struct unwrap<ReturnType(Args...) \
    FU2_MACRO_FULL_QUALIFIER(IS_CONST, IS_VOLATILE, IS_RVALUE)> \
    : unwrap_base< \
      signature<ReturnType(Args...)>, \
        qualifier<IS_CONST, IS_VOLATILE, IS_RVALUE> \
      > { }; \
    FU2_MACRO_EXPAND_LVALUE_ ## IS_RVALUE(IS_CONST, IS_VOLATILE)

FU2_MACRO_EXPAND_ALL(FU2_MACRO_DEFINE_SIGNATURE_UNWRAP)

#undef FU2_MACRO_DEFINE_SIGNATURE_UNWRAP
#undef FU2_MACRO_EXPAND_LVALUE_true
#undef FU2_MACRO_EXPAND_LVALUE_false

// Rounds the required size up to the alignment
template<std::size_t Size, std::size_t Alignment>
using round_up_to_alignment = std::integral_constant<std::size_t,
  (Size % Alignment == 0)
    ? Size
    : Size + (Alignment - (Size % Alignment))
>;

// Defines the required capacity which is needed to allocate an object
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
struct function_vtable<signature<ReturnType(Args...)>, Copyable> {
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
   : function_vtable<signature<ReturnType(Args...)>, false> {
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
inline void function_wrapper_noop(void* /*dest*/) { }

// Performs no operation on the given pointer.
inline void function_wrapper_noop2(void* /*dest*/, void* /*dest*/) { }

// Constructs a type T at the given destination with the given arguments.
template<typename T, typename... Args>
static void function_wrapper_construct(void* destination, Args... args) {
  new (destination) typename std::decay<T>::type(std::forward<Args>(args)...);
}

// Destructs a type T at the given destination.
template<typename T>
static void function_wrapper_destruct(void* destination) {
  static_cast<T*>(destination)->~T();
  (void)destination;
}

// Returns the required size of the type to allocate in-place.
template<typename T>
static std::size_t function_wrapper_required_size() {
  return required_capacity_to_allocate_inplace<T>::value;
}

// Returns a zero size.
inline std::size_t function_wrapper_zero_size() {
  return 0UL;
}

// Moves the given type at the target location to another one.
template<typename T>
static void function_wrapper_move(void* from, void* to) {
  function_wrapper_construct<T>(to, std::move(*static_cast<T*>(from)));
}

// Copies the given type at the target location to another one.
template<typename T>
static void function_wrapper_copy(void* from, void* to) {
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
  > { \
    static ReturnType invoke(void* target, Args&&... args) { \
      return FU2_MACRO_MOVE_IF(IS_RVALUE)(* static_cast< \
        T FU2_MACRO_NO_REF_QUALIFIER(IS_CONST, IS_VOLATILE) *>( \
          target))(std::forward<Args>(args)...); \
    } \
  };

FU2_MACRO_EXPAND_ALL(FU2_MACRO_DEFINE_CALL_OPERATOR)

#undef FU2_MACRO_DEFINE_CALL_OPERATOR

#if defined(FU2_NO_FUNCTIONAL_HEADER) && !defined(FU2_MACRO_DISABLE_EXCEPTIONS)
  struct bad_function_call : std::exception {
    bad_function_call() { }

    char const* what() const throw() override {
      return "bad function call";
    }
  };
#endif

template<typename /*Signature*/, bool /*Throws*/>
struct vtable_creator_of_empty_function;

template<typename ReturnType, typename... Args>
struct vtable_creator_of_empty_function<signature<ReturnType(Args...)>, true> {
  using common_vtable_t = function_vtable<
    signature<ReturnType(Args...)>,
    true
  >;

  // Throws an empty function call
  static ReturnType invoke(void*, Args&&...) {
#ifdef FU2_MACRO_DISABLE_EXCEPTIONS
    std::abort();
#elif !defined(FU2_NO_FUNCTIONAL_HEADER) \
      || !defined(FU2_MACRO_DISABLE_EXCEPTIONS)
    throw std::bad_function_call{};
#else
    throw bad_function_call{};
#endif
  }

  static common_vtable_t const* create_vtable() {
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
struct vtable_creator_of_empty_function<signature<ReturnType(Args...)>, false> {
  using common_vtable_t = function_vtable<
    signature<ReturnType(Args...)>,
    true
  >;

  // Non-Throwing empty function call
  static ReturnType invoke(void*, Args&&...) {
    std::abort();
  }

  static common_vtable_t const* create_vtable() {
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
                              Qualifier, true> {
  using common_vtable_t = function_vtable<
    signature<ReturnType(Args...)>,
    true
  >;

  static common_vtable_t const* create_vtable() {
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
                              Qualifier, false> {
  using common_vtable_t = function_vtable<
    signature<ReturnType(Args...)>,
    true
  >;

  static common_vtable_t const* create_vtable() {
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

struct initialize_functor_tag { };
struct copy_assign_storage_tag { };
struct move_assign_storage_tag { };

template<typename /*Signature*/, typename /*Qualifier*/, typename /*Config*/>
struct storage_t;

template<typename ReturnType, typename... Args,
         typename Qualifier, typename Config>
struct storage_t<signature<ReturnType(Args...)>, Qualifier, Config> {
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

  storage_t() {
    tidy();
  }

  explicit storage_t(storage_t const& right) {
    weak_copy_assign(right);
  }

  explicit storage_t(storage_t&& right) {
    weak_move_assign(std::move(right));
  }

  template<typename T>
  storage_t(initialize_functor_tag, T&& functor) {
    weak_allocate_object(std::forward<T>(functor));
  }

  template<typename T>
  storage_t(copy_assign_storage_tag, T const& right) {
    weak_copy_assign(right);
  }

  template<typename T>
  storage_t(move_assign_storage_tag, T&& right) {
    weak_move_assign(std::forward<T>(right));
  }

  storage_t& operator= (storage_t const& right) {
    weak_deallocate();
    weak_copy_assign(right);
    return *this;
  }

  storage_t& operator= (storage_t&& right) {
    weak_deallocate();
    weak_move_assign(std::move(right));
    return *this;
  }

  ~storage_t() {
    weak_deallocate();
  }

  // Private API
  void weak_deallocate() {
    _vtable->destruct(_impl);

    if (_impl != &_locale)
      std::free(_impl);
  }

  // Private API
  void deallocate() {
    weak_deallocate();
    tidy();
  }

  void tidy() {
    _vtable = vtable_creator_of_empty_function<
      signature<ReturnType(Args...)>, Config::is_throwing
    >::create_vtable();
    _impl = nullptr;
  }

  // Allocate in locale capacity.
  template<typename /*T*/>
  void allocate_space(std::true_type /*is_local_allocateable*/) {
    _impl = &_locale;
  }

  // Allocate on the heap.
  template<typename T>
  void allocate_space(std::false_type /*is_local_allocateable*/) {
    _impl = std::malloc(sizeof(T));
  }

  template<typename T>
  void weak_allocate_object(T functor) {
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

  // Private API
  template<typename RightConfig,
           typename std::enable_if<RightConfig::is_copyable>::type* = nullptr>
  void weak_copy_assign(storage_t<signature<ReturnType(Args...)>,
                        Qualifier, RightConfig> const& right) {
    _vtable = right._vtable;

    auto const required_size = right._vtable->required_size();
    if (right._impl == &right._locale && (Config::capacity >= required_size))
      _impl = &_locale;
    else
      _impl = std::malloc(required_size);

    right._vtable->copy(right._impl, _impl);
  }

  // Private API
  template<typename RightConfig>
  void weak_move_assign(storage_t<signature<ReturnType(Args...)>,
                        Qualifier, RightConfig>&& right) {
    _vtable = right._vtable;

    auto const required_size = right._vtable->required_size();
    if (right._impl == &right._locale) {
      if (Config::capacity >= required_size)
        _impl = &_locale;
      else
        _impl = std::malloc(required_size);

      right._vtable->move(right._impl, _impl);
      right.deallocate();
    }
    else {
      // Steal the ownership
      _impl = right._impl;
      right.tidy();
    }
  }

  bool empty() const { return _impl ? false : true; }

}; // struct storage_t

template <typename /*Fn*/>
struct call_operator;

#define FU2_MACRO_DEFINE_CALL_OPERATOR(IS_CONST, IS_VOLATILE, IS_RVALUE) \
  template<typename ReturnType, typename... Args, typename Config> \
  struct call_operator<function<signature<ReturnType(Args...)>, \
                                qualifier<IS_CONST, IS_VOLATILE, IS_RVALUE>, \
                                Config>> { \
    ReturnType operator()(Args... args) \
      FU2_MACRO_FULL_QUALIFIER(IS_CONST, IS_VOLATILE, IS_RVALUE) { \
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

FU2_MACRO_EXPAND_ALL(FU2_MACRO_DEFINE_CALL_OPERATOR)

#undef FU2_MACRO_DEFINE_CALL_OPERATOR

template<typename ReturnType, typename... Args,
         typename Qualifier, typename Config>
class function<signature<ReturnType(Args...)>, Qualifier, Config>
  : public call_operator<
      function<signature<ReturnType(Args...)>, Qualifier, Config>
    >,
    public signature<ReturnType(Args...)>,
    public copyable<Config::is_copyable> {
  template<typename, typename, typename>
  friend class function;

  friend struct call_operator<function>;

  // Is a true type if the given function is copyable correct to this.
  template<bool RightCopyable>
  using is_copyable_correct_to_this = is_copyable_correct<
    Config::is_copyable, RightCopyable
  >;

  // SFINAE helper to filter not invocable parameters T.
  template<typename T>
  using invocation_acceptor_t = typename invocation_acceptor<
    T, ReturnType(Args...), Qualifier, Config
  >::type;

  // Implementation storage
  storage_t<signature<ReturnType(Args...)>, Qualifier, Config> _storage;

public:
  /// Default constructor which constructs the function empty
  function() = default;

  /// Copy construction from another copyable function
  template<typename RightConfig,
           typename std::enable_if<
            is_copyable_correct_to_this<RightConfig::is_copyable>::value &&
            RightConfig::is_copyable
           >::type* = nullptr>
  function(function<signature<ReturnType(Args...)>,
                              Qualifier, RightConfig> const& right)
    : _storage(copy_assign_storage_tag{}, right._storage) { }

  /// Move construction from another function
  template<typename RightConfig,
           typename std::enable_if<
            is_copyable_correct_to_this<RightConfig::is_copyable>::value
           >::type* = nullptr>
  function(function<signature<ReturnType(Args...)>,
                              Qualifier, RightConfig>&& right)
    : _storage(move_assign_storage_tag{}, std::move(right._storage)) { }

  /// Construction from a functional object which overloads the `()` operator
  template<typename T,
           typename Acceptor = invocation_acceptor_t<T>>
  function(T functor)
    : _storage(initialize_functor_tag{},
               Acceptor::wrap(std::forward<T>(functor))) { }

  /// Empty constructs the function
  explicit function(std::nullptr_t)
    : _storage() { }

  /// Copy assigning from another copyable function
  template<typename RightConfig,
           typename std::enable_if<RightConfig::is_copyable>::type* = nullptr>
  function& operator= (function<signature<ReturnType(Args...)>,
                                          Qualifier, RightConfig> const& right) {
    _storage.weak_deallocate();
    _storage.weak_copy_assign(right._storage);
    return *this;
  }

  /// Move assigning from another function
  template<typename RightConfig,
           typename std::enable_if<
            is_copyable_correct_to_this<RightConfig::is_copyable>::value
           >::type* = nullptr>
  function& operator= (function<signature<ReturnType(Args...)>,
                                          Qualifier, RightConfig>&& right) {
    _storage.weak_deallocate();
    _storage.weak_move_assign(std::move(right._storage));
    return *this;
  }

  /// Move assigning from a functional object
  template<typename T,
           typename Acceptor = invocation_acceptor_t<T>>
  function& operator= (T functor) {
    _storage.weak_deallocate();
    _storage.weak_allocate_object(Acceptor::wrap(std::forward<T>(functor)));
    return *this;
  }

  /// Clears the function
  function& operator= (std::nullptr_t) {
    _storage.deallocate();
    return *this;
  }

  /// Returns true when the function is empty
  bool empty() const { return _storage.empty(); }

  /// Returns true when the function isn't empty
  explicit operator bool() const { return !empty(); }

  /// Assigns a new target, note that the allocator
  /// is ignored like in the common standard library implementations.
  template<typename T, typename Alloc,
           typename Acceptor = invocation_acceptor_t<T>>
  void assign(T&& function, Alloc /*alloc*/) {
    *this = Acceptor::wrap(std::forward<T>(function));
  }

  /// Swaps this function with the given function
  void swap(function& other) {
    if (&other == this)
      return;

    function cache = std::move(other);
    other = std::move(*this);
    *this = std::move(cache);
  }

  /// Swaps the left function with the right one
  friend void swap(function& left, function& right) {
    left.swap(right);
  }

  /// Calls the function target, returns the result when the function exists
  /// otherwise it throws a fu2::bad_function_call when exceptions are enabled.
  /// When exceptions are disabled std::abort is called.
  using call_operator<function>::operator();

}; // class function

template<typename ReturnType, typename... Args,
         typename Qualifier, typename Config>
bool operator== (function<signature<ReturnType(Args...)>,
                                    Qualifier, Config> const& f, std::nullptr_t) {
  return !bool(f);
}

template<typename ReturnType, typename... Args,
         typename Qualifier, typename Config>
bool operator!= (function<signature<ReturnType(Args...)>,
                                    Qualifier, Config> const& f, std::nullptr_t) {
  return bool(f);
}

template<typename ReturnType, typename... Args,
         typename Qualifier, typename Config>
bool operator== (std::nullptr_t, function<signature<ReturnType(Args...)>,
                                                    Qualifier, Config> const& f) {
  return !bool(f);
}

template<typename ReturnType, typename... Args,
         typename Qualifier, typename Config>
bool operator!= (std::nullptr_t, function<signature<ReturnType(Args...)>,
                                                    Qualifier, Config> const& f) {
  return bool(f);
}

// Internal size of an empty function object
using empty_size = std::integral_constant<std::size_t,
  sizeof(function<
    unwrap<void()>::signature,
    unwrap<void()>::qualifier,
    config<true, 0UL, true, false>>)
>;

// Default capacity for small functor optimization
using default_capacity = std::integral_constant<std::size_t,
  // Aim to size the function object to 32UL
  (empty_size::value < 32UL)
    ? (32UL - empty_size::value)
    : 16UL
>;

} /// inline namespace
} /// namespace detail

/// Adaptable function wrapper base for arbitrary functional types.
template<
  /// Defines the signature of the function wrapper
  typename Signature,
  /// Defines whether the function is copyable or not
  bool Copyable,
  /// Defines the internal capacity of the function
  /// for small functor optimization.
  std::size_t Capacity = detail::default_capacity::value,
  /// Defines whether the function throws an exception on empty function call,
  /// `std::abort` is called otherwise.
  bool Throwing = true,
  /// Defines whether the function allows assignments from a
  /// function with less arguments.
  bool PartialApplyable = false>
using function_base = detail::function<
  typename detail::unwrap<Signature>::signature,
  typename detail::unwrap<Signature>::qualifier,
  detail::config<Copyable, Capacity, Throwing, PartialApplyable>
>;

/// Copyable function wrapper for arbitrary functional types.
template<typename Signature>
using function = function_base<
  Signature,
  true
>;

/// Non copyable function wrapper for arbitrary functional types.
template<typename Signature>
using unique_function = function_base<
  Signature,
  false
>;

/// Exception type when invoking empty functional wrappers.
///
/// The exception type thrown through empty function calls
/// when the template parameter 'Throwing' is set to true (default).
#if defined(FU2_NO_FUNCTIONAL_HEADER) && !defined(FU2_MACRO_DISABLE_EXCEPTIONS)
  using detail::bad_function_call;
#endif

} /// namespace fu2

#undef FU2_MACRO_DISABLE_EXCEPTIONS
#undef FU2_MACRO_EXPECT
#undef FU2_MACRO_IF
#undef FU2_MACRO_IF_true
#undef FU2_MACRO_IF_false
#undef FU2_MACRO_MOVE_IF
#undef FU2_MACRO_MOVE_IF_true
#undef FU2_MACRO_MOVE_IF_false
#undef FU2_MACRO_NO_REF_QUALIFIER
#undef FU2_MACRO_FULL_QUALIFIER
#undef FU2_MACRO_EXPAND_ALL

#endif // FU2_INCLUDED_FUNCTION2_HPP__
