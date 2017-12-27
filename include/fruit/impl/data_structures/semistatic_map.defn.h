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

#ifndef SEMISTATIC_MAP_DEFN_H
#define SEMISTATIC_MAP_DEFN_H

#include <fruit/impl/data_structures/semistatic_map.h>

namespace fruit {
namespace impl {

template <typename Key, typename Value>
inline SemistaticMap<Key, Value>::HashFunction::HashFunction() : a(0), shift(0) {}

template <typename Key, typename Value>
inline typename SemistaticMap<Key, Value>::Unsigned SemistaticMap<Key, Value>::HashFunction::hash(Unsigned x) const {
  return (Unsigned)(a * x) >> shift;
}

template <typename Key, typename Value>
inline typename SemistaticMap<Key, Value>::Unsigned SemistaticMap<Key, Value>::hash(const Key& key) const {
  return hash_function.hash(std::hash<typename std::remove_cv<Key>::type>()(key));
}

} // namespace impl
} // namespace fruit

#endif // SEMISTATIC_MAP_DEFN_H
