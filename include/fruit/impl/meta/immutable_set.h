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

#ifndef FRUIT_META_IMMUTABLE_SET_H
#define FRUIT_META_IMMUTABLE_SET_H

#include <fruit/impl/fruit_assert.h>
#include <fruit/impl/meta/basics.h>
#include <fruit/impl/meta/vector.h>

namespace fruit {
namespace impl {
namespace meta {

// ImmutableSet ::= ConsImmutableSet<Ts...>

template <typename... Ts>
struct ConsImmutableSet : public Ts... {};

struct VectorToImmutableSet {
  template <typename V>
  struct apply;

  template <typename... Ts>
  struct apply<Vector<Ts...>> {
    using type = ConsImmutableSet<Ts...>;
  };
};

struct IsInImmutableSet {
  template <typename S, typename T>
  struct apply {
    using type = Bool<std::is_base_of<T, S>::value>;
  };
};

struct SizeOfImmutableSet {
  template <typename IS>
  struct apply;

  template <typename... Ts>
  struct apply<ConsImmutableSet<Ts...>> {
    using type = Int<sizeof...(Ts)>;
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_IMMUTABLE_SET_H
