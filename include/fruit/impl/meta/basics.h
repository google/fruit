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

struct If {};

template <typename MetaExpr>
struct DoEval;

// General case, meta-constant.
template <typename MetaExpr>
struct DoEval {
  using type = MetaExpr;
};

template <typename MetaFun, typename... MetaExprs>
struct DoEval<MetaFun(MetaExprs...)> {
  using type = typename DoEval<typename MetaFun::template apply<
      typename DoEval<MetaExprs>::type...
      >::type>::type;
};

// Similar to the previous specialization, but this will be selected when the function signature
// became a function pointer (this happens when a signature parameter is itself a signature).
template <typename MetaFun, typename... MetaExprs>
struct DoEval<MetaFun(*)(MetaExprs...)> {
  using type = typename DoEval<typename MetaFun::template apply<
      typename DoEval<MetaExprs>::type...
      >::type>::type;
};

template <typename MetaBool, typename ThenMetaExpr, typename ElseMetaExpr>
struct EvalIf {
  using type = typename DoEval<ThenMetaExpr>::type;
};

template <typename ThenMetaExpr, typename ElseMetaExpr>
struct EvalIf<Bool<false>, ThenMetaExpr, ElseMetaExpr> {
  using type = typename DoEval<ElseMetaExpr>::type;
};

template <typename CondMetaExpr, typename ThenMetaExpr, typename ElseMetaExpr>
struct DoEval<If(CondMetaExpr, ThenMetaExpr, ElseMetaExpr)> {
  using type = typename EvalIf<typename DoEval<CondMetaExpr>::type,
                               ThenMetaExpr,
                               ElseMetaExpr
                               >::type;
};

// Similar to the previous specialization, but this will be selected when the function signature
// became a function pointer (this happens when a signature parameter is itself a signature).
template <typename CondMetaExpr, typename ThenMetaExpr, typename ElseMetaExpr>
struct DoEval<If(*)(CondMetaExpr, ThenMetaExpr, ElseMetaExpr)> {
  using type = typename EvalIf<typename DoEval<CondMetaExpr>::type,
                               ThenMetaExpr,
                               ElseMetaExpr
                               >::type;
};

template <typename MetaExpr>
using Eval = typename DoEval<MetaExpr>::type;

// Call(F, Args...) is equivalent to F(Args...) in a metaexpression, except that Call(F, Args...)
// also works when F is a metaexpression.
struct Call {
  template <typename F, typename... Args>
  struct apply {
    using type = F(Args...);
  };
};

// UnwrapType<Type<T>> is T.
template <typename WrappedType>
using UnwrapType = typename WrappedType::type;

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
    using type = If(MetaExpr, 
                    If(MetaExpr2, And(MetaExprs...), Bool<false>),
                    Bool<false>);
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
    using type = If(MetaExpr, 
                    Bool<true>,
                    If(MetaExpr2, Bool<true>, Or(MetaExprs...)));
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

template <typename T>
using Id = T;


} // namespace meta
} // namespace impl
} // namespace fruit


#endif // FRUIT_META_BASICS_H
