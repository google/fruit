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

// None is used as "the nullptr of metaprogramming". E.g. when a function has no meaningful value to
// return, it can return None instead.
struct None {};

struct If {};

// PropagateError(E1, X) is equivalent to If(IsError(E1), E1, X).
// Note that X is evaluated lazily, as the If would do.
struct PropagateError {};

// Used to propagate an ErrorTag::apply<ErrorArgs...> up the instantiation chain, but without instantiating it right away, to allow shorter error stacktraces.
// Instantiating ErrorTag::apply<ErrorArgs...> must result in a static_assert error.
template <typename ErrorTag, typename... ErrorArgs>
struct Error {};

template <typename MetaExpr>
struct DoEval;

// General case, meta-constant.
template <typename MetaExpr>
struct DoEval {
#ifdef FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) { return true; }
  static_assert(static_warning(), "");
#endif
  using type = MetaExpr;
};

#ifdef FRUIT_EXTRA_DEBUG

// For debugging, we use a separate DoEvalFun so that we get longer (and more informative)
// instantiation traces.

template <typename MetaFun, typename... Params>
struct DoEvalFun {
  using type = typename DoEval<typename MetaFun::template apply<Params...>::type>::type;
};

template <typename MetaFun, typename... MetaExprs>
struct DoEval<MetaFun(MetaExprs...)> {
#ifdef FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) { return true; }
  static_assert(static_warning(), "");
#endif
  using type = typename DoEvalFun<MetaFun,
      typename DoEval<MetaExprs>::type...
      >::type;
};

// Similar to the previous specialization, but this will be selected when the function signature
// became a function pointer (this happens when a signature parameter is itself a signature).
template <typename MetaFun, typename... MetaExprs>
struct DoEval<MetaFun(*)(MetaExprs...)> {
#ifdef FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) { return true; }
  static_assert(static_warning(), "");
#endif
  using type = typename DoEvalFun<MetaFun,
      typename DoEval<MetaExprs>::type...
      >::type;
};

#else // FRUIT_EXTRA_DEBUG

template <typename MetaFun, typename... MetaExprs>
struct DoEval<MetaFun(MetaExprs...)> {
#ifdef FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) { return true; }
  static_assert(static_warning(), "");
#endif
  using type = typename DoEval<typename MetaFun::template apply<
      typename DoEval<MetaExprs>::type...
      >::type>::type;
};

// Similar to the previous specialization, but this will be selected when the function signature
// became a function pointer (this happens when a signature parameter is itself a signature).
template <typename MetaFun, typename... MetaExprs>
struct DoEval<MetaFun(*)(MetaExprs...)> {
#ifdef FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) { return true; }
  static_assert(static_warning(), "");
#endif
  using type = typename DoEval<typename MetaFun::template apply<
      typename DoEval<MetaExprs>::type...
      >::type>::type;
};

#endif // FRUIT_EXTRA_DEBUG


template <typename MetaBool, typename ThenMetaExpr, typename ElseMetaExpr>
struct EvalIf {
#ifdef FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) { return true; }
  static_assert(static_warning(), "");
#endif
  using type = typename DoEval<ThenMetaExpr>::type;
};

template <typename ThenMetaExpr, typename ElseMetaExpr>
struct EvalIf<Bool<false>, ThenMetaExpr, ElseMetaExpr> {
#ifdef FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) { return true; }
  static_assert(static_warning(), "");
#endif
  using type = typename DoEval<ElseMetaExpr>::type;
};

template <typename CondMetaExpr, typename ThenMetaExpr, typename ElseMetaExpr>
struct DoEval<If(CondMetaExpr, ThenMetaExpr, ElseMetaExpr)> {
#ifdef FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) { return true; }
  static_assert(static_warning(), "");
#endif
  using type = typename EvalIf<typename DoEval<CondMetaExpr>::type,
                               ThenMetaExpr,
                               ElseMetaExpr
                               >::type;
};

// Similar to the previous specialization, but this will be selected when the function signature
// became a function pointer (this happens when a signature parameter is itself a signature).
template <typename CondMetaExpr, typename ThenMetaExpr, typename ElseMetaExpr>
struct DoEval<If(*)(CondMetaExpr, ThenMetaExpr, ElseMetaExpr)> {
#ifdef FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) { return true; }
  static_assert(static_warning(), "");
#endif
  using type = typename EvalIf<typename DoEval<CondMetaExpr>::type,
                               ThenMetaExpr,
                               ElseMetaExpr
                               >::type;
};

template <typename T, typename ElseMetaExpr>
struct EvalPropagateError {
  using type = typename DoEval<ElseMetaExpr>::type;
};

template <typename ErrorTag, typename... ErrorArgs, typename ElseMetaExpr>
struct EvalPropagateError<Error<ErrorTag, ErrorArgs...>, ElseMetaExpr> {
  using type = Error<ErrorTag, ErrorArgs...>;
};

template <typename MaybeErrorMetaExpr, typename ElseMetaExpr>
struct DoEval<PropagateError(MaybeErrorMetaExpr, ElseMetaExpr)> {
#ifdef FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) { return true; }
  static_assert(static_warning(), "");
#endif
  using type = typename EvalPropagateError<typename DoEval<MaybeErrorMetaExpr>::type,
                                           ElseMetaExpr
                                           >::type;
};

// Similar to the previous specialization, but this will be selected when the function signature
// became a function pointer (this happens when a signature parameter is itself a signature).
template <typename MaybeErrorMetaExpr, typename ElseMetaExpr>
struct DoEval<PropagateError(*)(MaybeErrorMetaExpr, ElseMetaExpr)> {
#ifdef FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) { return true; }
  static_assert(static_warning(), "");
#endif
  using type = typename EvalPropagateError<typename DoEval<MaybeErrorMetaExpr>::type,
                                           ElseMetaExpr
                                           >::type;
};

template <typename MetaExpr>
using Eval = typename DoEval<MetaExpr>::type;

// Call(F, Args...) is equivalent to F(Args...) in a metaexpression, except that Call(F, Args...)
// also works when F is a metaexpression.
struct Call {
  template <typename F, typename... Args>
  struct apply : public F::template apply<Args...> {};
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

struct Fold {
  template <typename F, typename InitialValue, typename... Types>
  struct apply;
  
  template <typename F, typename InitialValue>
  struct apply<F, InitialValue> {
    using type = InitialValue;
  };
  
  template <typename F, typename InitialValue, typename T, typename... Types>
  struct apply<F, InitialValue, T, Types...> {
    using type = Fold(F, F(InitialValue, T), Types...);
  };
  
  // Optimized specialization, processing 3 values at a time.
  template <typename F, typename InitialValue, typename T0, typename T1, typename T2,
            typename... Types>
  struct apply<F, InitialValue, T0, T1, T2, Types...> {
    using type = Fold(F,
                      typename DoEval<typename F::template apply<
                      typename DoEval<typename F::template apply<
                      typename DoEval<typename F::template apply<
                        InitialValue,
                        T0>::type>::type,
                        T1>::type>::type,
                        T2>::type>::type,
                      Types...);
  };

  // Optimized specialization, processing 10 values at a time.
  template <typename F, typename InitialValue, typename T0, typename T1, typename T2, typename T3,
            typename T4, typename T5, typename T6, typename T7, typename T8, typename T9,
            typename... Types>
  struct apply<F, InitialValue, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, Types...> {
    using type = Fold(F,
                      typename DoEval<typename F::template apply<
                      typename DoEval<typename F::template apply<
                      typename DoEval<typename F::template apply<
                      typename DoEval<typename F::template apply<
                      typename DoEval<typename F::template apply<
                      typename DoEval<typename F::template apply<
                      typename DoEval<typename F::template apply<
                      typename DoEval<typename F::template apply<
                      typename DoEval<typename F::template apply<
                      typename DoEval<typename F::template apply<
                        InitialValue,
                        T0>::type>::type,
                        T1>::type>::type,
                        T2>::type>::type,
                        T3>::type>::type,
                        T4>::type>::type,
                        T5>::type>::type,
                        T6>::type>::type,
                        T7>::type>::type,
                        T8>::type>::type,
                        T9>::type>::type,
                      Types...);
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit


#endif // FRUIT_META_BASICS_H
