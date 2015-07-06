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
struct Error {};

template <typename... Types>
struct CheckIfError {
  using type = int;
};

template <typename ErrorTag, typename... ErrorArgs>
struct CheckIfError<Error<ErrorTag, ErrorArgs...>> {
  using type = typename ErrorTag::template apply<ErrorArgs...>;
};

// ConstructError(ErrorTag, Args...) returns Error<ErrorTag, Args...>.
// Never construct an Error<...> directly, using this metafunction makes debugging easier.
struct ConstructError {
  template <typename ErrorTag, typename... Args>
  struct apply {
#ifdef FRUIT_DEEP_TEMPLATE_INSTANTIATION_STACKTRACES_FOR_ERRORS
    static_assert(true || sizeof(typename CheckIfError<Error<ErrorTag, Args...>>::type), "");
#endif
    using type = Error<ErrorTag, Args...>;
  };
};

struct ConstructErrorWithArgVector {
  template <typename ErrorTag, typename ArgsVector, typename... OtherArgs>
  struct apply;
  
  template <typename ErrorTag, typename... Args, typename... OtherArgs>
  struct apply<ErrorTag, Vector<Args...>, OtherArgs...> {
    using type = ConstructError(ErrorTag, OtherArgs..., Args...);
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

// Extracts the first error in the given types.
struct ExtractFirstError {
  template <typename... Types>
  struct apply;
  
  template <typename Type, typename... Types>
  struct apply<Type, Types...> {
    using type = ExtractFirstError(Types...);
  };
  
  template <typename ErrorTag, typename... ErrorParams, typename... Types>
  struct apply<Error<ErrorTag, ErrorParams...>, Types...> {
    using type = Error<ErrorTag, ErrorParams...>;
  };
};

// Use as: Apply<ApplyAndPostponeFirstArgument<PropagateErrors, Params...>, Result>
// Use as: PropagateErrors(Result, Params...)
// If any type in Params... is an Error, the result is the first error. Otherwise, it's Result.
struct PropagateErrors {
  template <typename Result, typename... Params>
  struct apply {
    using type = If(Or(Id<IsError(Params)>...),
                    ExtractFirstError(Params...),
                    Result);
  };
};

// Use as CheckedCall(F, Arg1, Arg2).
// Similar to Apply, but if any argument is an Error<...> returns the first Error<...> argument
// instead of calling F.
struct CheckedCall {
  template <typename F, typename... Args>
  struct apply {
    using type = If(Or(Id<IsError(Args)>...),
                    ExtractFirstError(Args...),
                    F(Args...));
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit


#endif // FRUIT_META_ERRORS_H
