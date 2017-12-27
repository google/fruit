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

#ifndef FRUIT_META_ALGOS_H
#define FRUIT_META_ALGOS_H

#include <fruit/impl/fruit-config.h>
#include <fruit/impl/meta/immutable_map.h>

namespace fruit {
namespace impl {
namespace meta {

// We need a different (slower) implementation to workaround a Clang bug:
// https://llvm.org/bugs/show_bug.cgi?id=25669
// TODO: remove this once that bug is fixed (for the appropriate Clang versions).
#if FRUIT_HAS_CLANG_ARBITRARY_OVERLOAD_RESOLUTION_BUG

struct HasDuplicatesHelper {
  template <typename... Types>
  struct apply {
    using type = Bool<false>;
  };

  template <typename Type, typename... Types>
  struct apply<Type, Types...> {
    using type = Or(StaticOr<std::is_same<Type, Types>::value...>, Id<HasDuplicatesHelper(Types...)>);
  };
};

struct HasDuplicates {
  template <typename V>
  struct apply;

  template <typename... Types>
  struct apply<Vector<Types...>> {
    using type = HasDuplicatesHelper(Types...);
  };
};

#else // !FRUIT_HAS_CLANG_ARBITRARY_OVERLOAD_RESOLUTION_BUG

// Checks if the given Vector has duplicated types.
struct HasDuplicates {
  template <typename V>
  struct apply;

  template <typename... Types>
  struct apply<Vector<Types...>> {
    using M = VectorsToImmutableMap(Vector<Types...>, GenerateIntSequence(Int<sizeof...(Types)>));
    using type = Not(StaticAnd<Eval<ImmutableMapContainsKey(M, Types)>::value...>);
  };
};

#endif // FRUIT_HAS_CLANG_ARBITRARY_OVERLOAD_RESOLUTION_BUG

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_ALGOS_H
