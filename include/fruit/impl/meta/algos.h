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

#include "immutable_map.h"

namespace fruit {
namespace impl {
namespace meta {

// Checks if the given Vector has duplicated types.
struct HasDuplicates {
  template <typename V>
  struct apply;
  
  template <typename... Types>
  struct apply<Vector<Types...>> {
    // This depends on the implementation of FindInMap: when a key is duplicate in the map,
    // FindInMap can't find a (single) value to map it to and returns None.
    using M = VectorsToImmutableMap(Vector<Types...>, GenerateIntSequence(Int<sizeof...(Types)>));
    using type = Not(StaticAnd<Eval<ImmutableMapContainsKey(M, Types)>::value
                               ...>);
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_ALGOS_H
