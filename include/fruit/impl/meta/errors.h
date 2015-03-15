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

#ifndef FRUIT_META_ERRORS_H
#define FRUIT_META_ERRORS_H

#include "basics.h"
#include "logical_operations.h"
#include "vector.h"

namespace fruit {
namespace impl {
namespace meta {

// Used to propagate an ErrorTag::apply<ErrorArgs...> up the instantiation chain, but without instantiating it right away, to allow shorter error stacktraces.
// Instantiating ErrorTag::apply<ErrorArgs...> must result in a static_assert error.
template <typename ErrorTag, typename... ErrorArgs>
struct Error;

struct ConstructErrorWithArgVector {
  template <typename ErrorTag, typename ArgsVector, typename... OtherArgs>
  struct apply;
  
  template <typename ErrorTag, typename... Args, typename... OtherArgs>
  struct apply<ErrorTag, Vector<Args...>, OtherArgs...> {
    using type = Error<ErrorTag, OtherArgs..., Args...>;
  };
};

struct IsError {
  template <typename... Types>
  struct apply {
    using type = Bool<false>;
  };

  template <typename ErrorTag, typename... ErrorArgs>
  struct apply<Error<ErrorTag, ErrorArgs...>> {
    using type = Bool<true>;
  };
};

template <typename... Types>
struct CheckIfError {
  using type = int;
};

template <typename ErrorTag, typename... ErrorArgs>
struct CheckIfError<Error<ErrorTag, ErrorArgs...>> {
  using type = typename ErrorTag::template apply<ErrorArgs...>;
};

// Extracts the first error in the given types.
struct ExtractFirstError {
  template <typename... Types>
  struct apply;
  
  template <typename Type, typename... Types>
  struct apply<Type, Types...> {
    using type = Apply<ExtractFirstError, Types...>;
  };
  
  template <typename ErrorTag, typename... ErrorParams, typename... Types>
  struct apply<Error<ErrorTag, ErrorParams...>, Types...> {
    using type = Error<ErrorTag, ErrorParams...>;
  };
};

// Use as: Apply<ApplyAndPostponeFirstArgument<PropagateErrors, Params...>, Result>
// If any type in Params... is an Error, the result is the first error. Otherwise, it's Result.
struct PropagateErrors {
  template <typename Result, typename... Params>
  struct apply {
    using type = Eval<Conditional<Lazy<Bool<StaticOr<Apply<IsError, Params>::value...>::value>>,
                                  Apply<LazyFunctor<ExtractFirstError>, Lazy<Params>...>,
                                  Lazy<Result>
                                  >>;
  };
};

// Use as CheckedApply<MyMetafunction, Arg1, Arg2>.
// Similar to Apply, but if any argument is an Error<...> returns the first Error<...> argument
// instead of calling F.
template <typename F, typename... Args>
using CheckedApply = Eval<Conditional<Lazy<Bool<StaticOr<Apply<IsError, Args>::value...>::value>>,
                                      Apply<LazyFunctor<ExtractFirstError>, Lazy<Args>...>,
                                      Apply<LazyFunctor<F>, Lazy<Args>...>
                                      >>;

} // namespace meta
} // namespace impl
} // namespace fruit


#endif // FRUIT_META_ERRORS_H
