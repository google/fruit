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

#ifndef FRUIT_META_EVAL_H
#define FRUIT_META_EVAL_H

#include <fruit/impl/meta/basics.h>
#include <fruit/impl/meta/errors.h>
#include <fruit/impl/meta/logical_operations.h>

#include <functional>

namespace fruit {
namespace impl {
namespace meta {

template <typename MetaExpr>
struct DoEval;

// General case, meta-constant.
template <typename MetaExpr>
struct DoEval {
#if FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) {
    return true;
  }
  static_assert(static_warning(), "");
#endif
  using type = MetaExpr;
};

template <typename Type>
struct SimpleIsError {
  static constexpr bool value = false;
};
template <typename ErrorTag, typename... ErrorArgs>
struct SimpleIsError<Error<ErrorTag, ErrorArgs...>> {
  static constexpr bool value = true;
};

#if FRUIT_EXTRA_DEBUG

// For debugging, we use a separate DoEvalFun so that we get longer (and more informative)
// instantiation traces.

template <typename MetaFun, typename... Params>
struct DoEvalFun {
  using type =
      typename DoEval<typename std::conditional<StaticOr<SimpleIsError<Params>::value...>::value, ExtractFirstError,
                                                MetaFun>::type::template apply<Params...>::type>::type;
};

template <typename MetaFun, typename... MetaExprs>
struct DoEval<MetaFun(MetaExprs...)> {
#if FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) {
    return true;
  }
  static_assert(static_warning(), "");
#endif
  using type = typename DoEvalFun<MetaFun, typename DoEval<MetaExprs>::type...>::type;
};

// Similar to the previous specialization, but this will be selected when the function signature
// became a function pointer (this happens when a signature parameter is itself a signature).
template <typename MetaFun, typename... MetaExprs>
struct DoEval<MetaFun (*)(MetaExprs...)> {
#if FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) {
    return true;
  }
  static_assert(static_warning(), "");
#endif
  using type = typename DoEvalFun<MetaFun, typename DoEval<MetaExprs>::type...>::type;
};

#else // FRUIT_EXTRA_DEBUG

template <typename MetaFun, typename... MetaExprs>
struct DoEval<MetaFun(MetaExprs...)> {
#if FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) {
    return true;
  }
  static_assert(static_warning(), "");
#endif
  using type = typename DoEval<typename std::conditional<
      StaticOr<SimpleIsError<typename DoEval<MetaExprs>::type>::value...>::value, ExtractFirstError,
      MetaFun>::type::template apply<typename DoEval<MetaExprs>::type...>::type>::type;
};

// Similar to the previous specialization, but this will be selected when the function signature
// became a function pointer (this happens when a signature parameter is itself a signature).
template <typename MetaFun, typename... MetaExprs>
struct DoEval<MetaFun (*)(MetaExprs...)> {
#if FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) {
    return true;
  }
  static_assert(static_warning(), "");
#endif
  using type = typename DoEval<typename std::conditional<
      StaticOr<SimpleIsError<typename DoEval<MetaExprs>::type>::value...>::value, ExtractFirstError,
      MetaFun>::type::template apply<typename DoEval<MetaExprs>::type...>::type>::type;
};

#endif // FRUIT_EXTRA_DEBUG

template <typename ExprResult, typename ErrorTag, typename Handler>
struct EvalCatch {
  using type = ExprResult;
};

template <typename CaughtErrorTag, typename... ErrorArgs, typename Handler>
struct EvalCatch<Error<CaughtErrorTag, ErrorArgs...>, CaughtErrorTag, Handler> {
  using type =
      typename DoEval<typename DoEval<Handler>::type::template apply<Error<CaughtErrorTag, ErrorArgs...>>::type>::type;
};

template <typename ExprResult, typename Handler>
struct EvalCatchAll {
  using type = ExprResult;
};

template <typename CaughtErrorTag, typename... ErrorArgs, typename Handler>
struct EvalCatchAll<Error<CaughtErrorTag, ErrorArgs...>, Handler> {
  using type =
      typename DoEval<typename DoEval<Handler>::type::template apply<Error<CaughtErrorTag, ErrorArgs...>>::type>::type;
};

template <typename Expr, typename ErrorTag, typename Handler>
struct DoEval<Catch(Expr, ErrorTag, Handler)> {
  using type = typename EvalCatch<typename DoEval<Expr>::type, typename DoEval<ErrorTag>::type, Handler>::type;
};

template <typename Expr, typename ErrorTag, typename Handler>
struct DoEval<Catch (*)(Expr, ErrorTag, Handler)> {
  using type = typename EvalCatch<typename DoEval<Expr>::type, typename DoEval<ErrorTag>::type, Handler>::type;
};

template <typename Expr, typename Handler>
struct DoEval<CatchAll(Expr, Handler)> {
  using type = typename EvalCatchAll<typename DoEval<Expr>::type, Handler>::type;
};

template <typename Expr, typename Handler>
struct DoEval<CatchAll (*)(Expr, Handler)> {
  using type = typename EvalCatchAll<typename DoEval<Expr>::type, Handler>::type;
};

template <typename MetaBool, typename ThenMetaExpr, typename ElseMetaExpr>
struct EvalIf;

template <typename ErrorTag, typename... ErrorArgs, typename ThenMetaExpr, typename ElseMetaExpr>
struct EvalIf<Error<ErrorTag, ErrorArgs...>, ThenMetaExpr, ElseMetaExpr> {
  using type = Error<ErrorTag, ErrorArgs...>;
};

template <typename ThenMetaExpr, typename ElseMetaExpr>
struct EvalIf<Bool<true>, ThenMetaExpr, ElseMetaExpr> {
#if FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) {
    return true;
  }
  static_assert(static_warning(), "");
#endif
  using type = typename DoEval<ThenMetaExpr>::type;
};

template <typename ThenMetaExpr, typename ElseMetaExpr>
struct EvalIf<Bool<false>, ThenMetaExpr, ElseMetaExpr> {
#if FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) {
    return true;
  }
  static_assert(static_warning(), "");
#endif
  using type = typename DoEval<ElseMetaExpr>::type;
};

template <typename CondMetaExpr, typename ThenMetaExpr, typename ElseMetaExpr>
struct DoEval<If(CondMetaExpr, ThenMetaExpr, ElseMetaExpr)> {
#if FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) {
    return true;
  }
  static_assert(static_warning(), "");
#endif
  using type = typename EvalIf<typename DoEval<CondMetaExpr>::type, ThenMetaExpr, ElseMetaExpr>::type;
};

// Similar to the previous specialization, but this will be selected when the function signature
// became a function pointer (this happens when a signature parameter is itself a signature).
template <typename CondMetaExpr, typename ThenMetaExpr, typename ElseMetaExpr>
struct DoEval<If (*)(CondMetaExpr, ThenMetaExpr, ElseMetaExpr)> {
#if FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) {
    return true;
  }
  static_assert(static_warning(), "");
#endif
  using type = typename EvalIf<typename DoEval<CondMetaExpr>::type, ThenMetaExpr, ElseMetaExpr>::type;
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
#if FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) {
    return true;
  }
  static_assert(static_warning(), "");
#endif
  using type = typename EvalPropagateError<typename DoEval<MaybeErrorMetaExpr>::type, ElseMetaExpr>::type;
};

// Similar to the previous specialization, but this will be selected when the function signature
// became a function pointer (this happens when a signature parameter is itself a signature).
template <typename MaybeErrorMetaExpr, typename ElseMetaExpr>
struct DoEval<PropagateError (*)(MaybeErrorMetaExpr, ElseMetaExpr)> {
#if FRUIT_TRACE_INSTANTIATIONS
  constexpr static bool static_warning() __attribute__((deprecated("static_warning"))) {
    return true;
  }
  static_assert(static_warning(), "");
#endif
  using type = typename EvalPropagateError<typename DoEval<MaybeErrorMetaExpr>::type, ElseMetaExpr>::type;
};

template <typename MetaExpr>
using Eval = typename DoEval<MetaExpr>::type;

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_EVAL_H
