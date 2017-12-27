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

#ifndef FRUIT_META_LIST_H
#define FRUIT_META_LIST_H

#include <fruit/impl/meta/basics.h>
#include <fruit/impl/meta/logical_operations.h>
#include <fruit/impl/meta/numeric_operations.h>
#include <functional>

namespace fruit {
namespace impl {
namespace meta {

// List ::= EmptyList | Cons<T, List>

struct EmptyList {};

template <typename T, typename Tail>
struct Cons {};

// TODO: Consider inlining to improve performance.
// If L is a list containing T1,...,Tn this calculates F(InitialValue, F(T1, F(..., F(Tn) ...))).
struct FoldList {
  template <typename L, typename F, typename InitialValue>
  struct apply;

  template <typename F, typename InitialValue>
  struct apply<EmptyList, F, InitialValue> {
    using type = InitialValue;
  };

  template <typename Head, typename Tail, typename F, typename InitialValue>
  struct apply<Cons<Head, Tail>, F, InitialValue> {
    using type = FoldList(Tail, F, F(InitialValue, Head));
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_LIST_H
