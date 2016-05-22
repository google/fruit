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
inline HashSet<T> createHashSet() {
  return createHashSet<T>(10);
}

template <typename T>
inline HashSet<T> createHashSet(size_t capacity) {
  HashSet<T> result(capacity, std::hash<T>(), std::equal<T>());
  return std::move(result);
}

template <typename T>
inline HashSetWithGreedyAllocator<T> createHashSet(GreedyAllocatorStorage& allocatorStorage) {
  return createHashSet<T>(10, allocatorStorage);
}

template <typename T>
inline HashSetWithGreedyAllocator<T> createHashSet(size_t capacity, GreedyAllocatorStorage& allocatorStorage) {
  HashSetWithGreedyAllocator<T> result(capacity, std::hash<T>(), std::equal_to<T>(), GreedyAllocator<T>(allocatorStorage));
  return std::move(result);
}

template <typename Key, typename Value>
inline HashMap<Key, Value> createHashMap() {
  return createHashMap<Key, Value>(10);
}

template <typename Key, typename Value>
inline HashMap<Key, Value> createHashMap(size_t capacity) {
  HashMap<Key, Value> result(capacity);
  return std::move(result);
}

template <typename Key, typename Value>
inline HashMapWithGreedyAllocator<Key, Value> createHashMap(GreedyAllocatorStorage& allocatorStorage) {
  return createHashMap<Key, Value>(10, allocatorStorage);
}

template <typename Key, typename Value>
inline HashMapWithGreedyAllocator<Key, Value> createHashMap(size_t capacity, GreedyAllocatorStorage& allocatorStorage) {
  HashMapWithGreedyAllocator<Key, Value> result(capacity, std::hash<Key>(), std::equal_to<Key>(), GreedyAllocator<std::pair<const Key, Value>>(allocatorStorage));
  return std::move(result);
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_SPARSEHASH_HELPERS_DEFN_H
