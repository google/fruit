/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FRUIT_META_BASICS_H
#define FRUIT_META_BASICS_H

#include <functional>

namespace fruit {
namespace impl {
namespace meta {

template <typename T>
struct Type {
  using type = T;
};

template <bool b>
struct Bool {
  static constexpr bool value = b;
};

template <int n>
struct Int {
  static constexpr int value = n;
};

// This was added to workaround a bug in MSVC 2017 15.5, that crashes when expanding Indexes::value... in some cases
// (where Indexes is a template parameter pack of Int<...> types).
// TODO: Remove this once MSVC 2017 is fixed and the fix has been out for some time.
template <typename N>
constexpr int getIntValue() {
  return N::value;
}

// None is used as "the nullptr of metaprogramming". E.g. when a function has no meaningful value to
// return, it can return None instead.
struct None {};

struct If {};

// PropagateError(E, X) evaluates E then X. The result is X's result, but if E returns an error,
// that's the result instead.
struct PropagateError {};

// Used to propagate an ErrorTag::apply<ErrorArgs...> up the instantiation chain, but without instantiating it right
// away, to allow shorter error stacktraces.
// Instantiating ErrorTag::apply<ErrorArgs...> must result in a static_assert error.
template <typename ErrorTag, typename... ErrorArgs>
struct Error {};

// Use as Catch(ExpressionThatMightThrow, ErrorTag, Handler)
// Handler(Error<ErrorTag, ErrorArgs...>) is called if ExpressionThatMightThrow throws ErrorTag.
struct Catch {};

// Use as CatchAll(ExpressionThatMightThrow, Handler)
// Handler(Error<ErrorTag, ErrorArgs...>) is called if ExpressionThatMightThrow throws any error.
struct CatchAll {};

// Call(F, Args...) is equivalent to F(Args...) in a metaexpression, except that Call(F, Args...)
// also works when F is a metaexpression.
struct Call {
  template <typename F, typename... Args>
  struct apply : public F::template apply<Args...> {};
};

// UnwrapType<Type<T>> is T.
template <typename WrappedType>
using UnwrapType = typename WrappedType::type;

// MSVC 14 has trouble specializing alias templates using expanded pack elements.
// This is a known issue:
// https://stackoverflow.com/questions/43411542/metaprogramming-failed-to-specialize-alias-template
// The workaround is just to use a struct directly.
// typename TypeUnwrapper<Type<T>>::type is T.
template <typename WrappedType>
struct TypeUnwrapper {
  using type = UnwrapType<WrappedType>;
};

// Logical And with short-circuit evaluation.
struct And {
  template <typename... MetaExprs>
  struct apply {
    using type = Bool<true>;
  };

  template <typename MetaExpr>
  struct apply<MetaExpr> {
    using type = MetaExpr;
  };

  template <typename MetaExpr, typename MetaExpr2>
  struct apply<MetaExpr, MetaExpr2> {
    using type = If(MetaExpr, MetaExpr2, Bool<false>);
  };

  template <typename MetaExpr, typename MetaExpr2, typename... MetaExprs>
  struct apply<MetaExpr, MetaExpr2, MetaExprs...> {
    using type = If(MetaExpr, If(MetaExpr2, And(MetaExprs...), Bool<false>), Bool<false>);
  };
};

// Logical Or with short-circuit evaluation.
struct Or {
  template <typename... MetaExprs>
  struct apply {
    using type = Bool<false>;
  };

  template <typename MetaExpr>
  struct apply<MetaExpr> {
    using type = MetaExpr;
  };

  template <typename MetaExpr, typename MetaExpr2>
  struct apply<MetaExpr, MetaExpr2> {
    using type = If(MetaExpr, Bool<true>, MetaExpr2);
  };

  template <typename MetaExpr, typename MetaExpr2, typename... MetaExprs>
  struct apply<MetaExpr, MetaExpr2, MetaExprs...> {
    using type = If(MetaExpr, Bool<true>, If(MetaExpr2, Bool<true>, Or(MetaExprs...)));
  };
};

// Call(Call(DeferArgs(F), Args...), MoreArgs...)
//
// is equivalent to:
// Result = F(Args..., MoreArgs...)
//
// Note that you can't write:
// DeferArgs(F)(Args...)(MoreArgs...)
//
// Because Call must be used to call metafunctions that are metaexpressions.
struct DeferArgs {
  template <typename F>
  struct apply {
    struct type {
      template <typename... Args>
      struct apply {
        struct type {
          template <typename... MoreArgs>
          struct apply {
            using type = F(Args..., MoreArgs...);
          };
        };
      };
    };
  };
};

// Call(PartialCall(F, Args...), MoreArgs...)
//
// is equivalent to:
// Result = F(Args..., MoreArgs...)
//
// Note that you can't write:
// PartialCall(F, Args...)(MoreArgs...)
//
// Because Call must be used to call metafunctions that are metaexpressions.
struct PartialCall {
  template <typename F, typename... Args>
  struct apply {
    struct type {
      template <typename... MoreArgs>
      struct apply {
        using type = F(Args..., MoreArgs...);
      };
    };
  };
};

struct IsSame {
  template <typename T, typename U>
  struct apply {
    using type = Bool<false>;
  };

  template <typename T>
  struct apply<T, T> {
    using type = Bool<true>;
  };
};

struct Not {
  template <typename B>
  struct apply {
    using type = Bool<!B::value>;
  };
};

struct IsNone {
  template <typename T>
  struct apply {
    using type = Bool<false>;
  };
};

template <>
struct IsNone::apply<None> {
  using type = Bool<true>;
};

template <typename T>
using Id = T;

struct Identity {
  template <typename T>
  struct apply {
    using type = T;
  };
};

template <typename T>
struct DebugTypeHelper {
  static_assert(sizeof(T*) * 0 != 0, "");
  using type = T;
};

template <typename T>
using DebugType = typename DebugTypeHelper<T>::type;

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_BASICS_H
