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

#include <fruit/impl/meta/basics.h>
#include <fruit/impl/meta/logical_operations.h>

namespace fruit {
namespace impl {
namespace meta {

template <typename T>
struct CheckIfError {
  using type = T;
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
#if FRUIT_DEEP_TEMPLATE_INSTANTIATION_STACKTRACES_FOR_ERRORS
    static_assert(true || sizeof(typename CheckIfError<Error<ErrorTag, UnwrapType<Args>...>>::type), "");
#endif
    using type = Error<ErrorTag, typename TypeUnwrapper<Args>::type...>;
  };
};

// Extracts the first error in the given types.
struct ExtractFirstError {
  template <typename... Types>
  struct apply;

  template <typename Type, typename... Types>
  struct apply<Type, Types...> : public apply<Types...> {};

  template <typename ErrorTag, typename... ErrorParams, typename... Types>
  struct apply<Error<ErrorTag, ErrorParams...>, Types...> {
    using type = Error<ErrorTag, ErrorParams...>;
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_ERRORS_H
