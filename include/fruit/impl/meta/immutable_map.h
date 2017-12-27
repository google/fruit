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

#ifndef FRUIT_META_IMMUTABLE_MAP_H
#define FRUIT_META_IMMUTABLE_MAP_H

#include <fruit/impl/meta/basics.h>
#include <fruit/impl/meta/immutable_set.h>
#include <fruit/impl/meta/pair.h>
#include <fruit/impl/meta/set.h>

namespace fruit {
namespace impl {
namespace meta {

// ImmutableMap ::= ImmutableSet<Pair<Key1, Value1>, ..., Pair<KeyN, ValueN>>

struct VectorsToImmutableMap {
  template <typename KeyVector, typename ValueVector>
  struct apply;

  template <typename... Keys, typename... Values>
  struct apply<Vector<Keys...>, Vector<Values...>> {
    using type = ConsImmutableSet<Pair<Keys, Values>...>;
  };
};

struct VectorToImmutableMap {
  template <typename PairVector>
  struct apply;

  template <typename... Pairs>
  struct apply<Vector<Pairs...>> {
    using type = ConsImmutableSet<Pairs...>;
  };
};

struct IsInImmutableMap {
  template <typename S, typename T>
  struct apply {
    using type = Bool<std::is_base_of<T, S>::value>;
  };
};

struct FindInImmutableMap {
  template <typename M, typename T>
  struct apply {
    template <typename Value>
    static Value f(Pair<T, Value>*);

    static None f(void*);

    using type = decltype(f((M*)nullptr));
  };
};

struct ImmutableMapContainsKey {
  template <typename M, typename T>
  struct apply {
    using type = Not(IsNone(FindInImmutableMap(M, T)));
  };
};

struct GetImmutableMapKeys {
  template <typename M>
  struct apply;

  template <typename... Pairs>
  struct apply<ConsImmutableSet<Pairs...>> {
    using type = Vector<typename Pairs::First...>;
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_IMMUTABLE_MAP_H
