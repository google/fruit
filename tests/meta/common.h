
#ifndef FRUIT_TESTS_META_COMMON_H
#define FRUIT_TESTS_META_COMMON_H

#define FRUIT_IN_META_TEST 1

#include <fruit/impl/injection_debug_errors.h>
#include <fruit/impl/injection_errors.h>
#include <fruit/impl/meta/basics.h>
#include <fruit/impl/meta/errors.h>
#include <fruit/impl/meta/immutable_map.h>
#include <fruit/impl/meta/set.h>
#include <fruit/impl/meta/vector.h>

using namespace std;
using namespace fruit;
using namespace fruit::impl;
using namespace fruit::impl::meta;

template <typename T, typename U>
struct DifferentError {
  static_assert(AlwaysFalse<T>::value, "T and U are different, but should have been equal/equivalent.");
};

template <typename T, typename U>
struct SameError {
  static_assert(AlwaysFalse<T>::value, "T and U are equal/equivalent but should have been different.");
};

struct DifferentErrorTag {
  template <typename T, typename U>
  using apply = DifferentError<T, U>;
};

struct SameErrorTag {
  template <typename T, typename U>
  using apply = SameError<T, U>;
};

template <typename... Types>
using ToSet = Vector<Types...>;

struct ConstructErrorWithoutUnwrapping {
  template <typename ErrorTag, typename... Args>
  struct apply {
    using type = ConstructError(ErrorTag, Type<Args>...);
  };
};

using True = Bool<true>;
using False = Bool<false>;

#undef Assert

#define Assert(...) static_assert(Eval<__VA_ARGS__>::value, "")
#define AssertNot(...) Assert(Not(__VA_ARGS__))
#define AssertSame(...)                                                                                                \
  static_assert(                                                                                                       \
      true ||                                                                                                          \
          sizeof(                                                                                                      \
              typename CheckIfError<Eval<If(IsSame(__VA_ARGS__), True,                                                 \
                                            ConstructErrorWithoutUnwrapping(DifferentErrorTag, __VA_ARGS__))>>::type), \
      "")
#define AssertSameType(...)                                                                                            \
  static_assert(                                                                                                       \
      true || sizeof(typename CheckIfError<                                                                            \
                     Eval<If(IsSame(__VA_ARGS__), True, ConstructError(DifferentErrorTag, __VA_ARGS__))>>::type),      \
      "")
#define AssertSameSet(...)                                                                                             \
  static_assert(                                                                                                       \
      true || sizeof(typename CheckIfError<                                                                            \
                     Eval<If(IsSameSet(__VA_ARGS__), True, ConstructError(DifferentErrorTag, __VA_ARGS__))>>::type),   \
      "")
#define AssertSameProof(...)                                                                                           \
  static_assert(true || sizeof(typename CheckIfError<Eval<If(IsProofTreeEqualTo(__VA_ARGS__), True,                    \
                                                             ConstructError(DifferentErrorTag, __VA_ARGS__))>>::type), \
                "")
#define AssertSameForest(...)                                                                                          \
  static_assert(true || sizeof(typename CheckIfError<Eval<CheckForestEqualTo(__VA_ARGS__)>>::type), "")
#define AssertNotSame(...)                                                                                             \
  static_assert(                                                                                                       \
      true ||                                                                                                          \
          sizeof(typename CheckIfError<Eval<If(Not(IsSame(__VA_ARGS__)), True,                                         \
                                               ConstructErrorWithoutUnwrapping(SameErrorTag, __VA_ARGS__))>>::type),   \
      "")
#define AssertNotSameType(...)                                                                                         \
  static_assert(                                                                                                       \
      true || sizeof(typename CheckIfError<                                                                            \
                     Eval<If(Not(IsSame(__VA_ARGS__)), True, ConstructError(SameErrorTag, __VA_ARGS__))>>::type),      \
      "")
#define AssertNotSameProof(...)                                                                                        \
  static_assert(true || sizeof(typename CheckIfError<Eval<If(Not(IsProofTreeEqualTo(__VA_ARGS__)), True,               \
                                                             ConstructError(SameErrorTag, __VA_ARGS__))>>::type),      \
                "")
#define AssertNotSameForest(...)                                                                                       \
  static_assert(                                                                                                       \
      true ||                                                                                                          \
          sizeof(typename CheckIfError<                                                                                \
                 Eval<If(Not(IsForestEqualTo(__VA_ARGS__)), True, ConstructError(SameErrorTag, __VA_ARGS__))>>::type), \
      "")

#endif // FRUIT_TESTS_META_COMMON_H
