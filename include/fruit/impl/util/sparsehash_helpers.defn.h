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

#ifndef FRUIT_SPARSEHASH_HELPERS_DEFN_H
#define FRUIT_SPARSEHASH_HELPERS_DEFN_H

#include <fruit/impl/util/sparsehash_helpers.h>

namespace fruit {
namespace impl {

template <typename T>
inline HashSet<T> createHashSet(T invalidValue1, T invalidValue2) {
  return createHashSet<T>(10, invalidValue1, invalidValue2);
}

template <typename T>
inline HashSet<T> createHashSet(size_t capacity, T invalidValue1, T invalidValue2) {
  HashSet<T> result(capacity, std::hash<T>());
  (void)invalidValue1;
  (void)invalidValue2;
  //result.set_empty_key(invalidValue1);
  //result.set_deleted_key(invalidValue2);
  return std::move(result);
}

template <typename Key, typename Value>
inline HashMap<Key, Value> createHashMap(Key invalidKey1, Key invalidKey2) {
  return createHashMap<Key, Value>(10, invalidKey1, invalidKey2);
}

template <typename Key, typename Value>
inline HashMap<Key, Value> createHashMap(size_t capacity, Key invalidKey1, Key invalidKey2) {
  HashMap<Key, Value> result(capacity, std::hash<Key>());
  (void)invalidKey1;
  (void)invalidKey2;
  //result.set_empty_key(invalidKey1);
  //result.set_deleted_key(invalidKey2);
  return std::move(result);
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_SPARSEHASH_HELPERS_DEFN_H
