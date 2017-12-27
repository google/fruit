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

#ifndef FRUIT_HASH_HELPERS_DEFN_H
#define FRUIT_HASH_HELPERS_DEFN_H

#include <fruit/impl/meta/vector.h>
#include <fruit/impl/util/hash_helpers.h>

namespace fruit {
namespace impl {

template <typename T>
inline HashSet<T> createHashSet() {
  return createHashSet<T>(10);
}

template <typename T>
inline HashSet<T> createHashSet(size_t capacity) {
  return HashSet<T>(capacity, std::hash<T>());
}

template <typename T>
inline HashSetWithArenaAllocator<T> createHashSetWithArenaAllocator(size_t capacity, MemoryPool& memory_pool) {
  return HashSetWithArenaAllocator<T>(capacity, std::hash<T>(), std::equal_to<T>(), ArenaAllocator<T>(memory_pool));
}

template <typename T, typename Hasher, typename EqualityComparator>
inline HashSetWithArenaAllocator<T, Hasher, EqualityComparator>
createHashSetWithArenaAllocatorAndCustomFunctors(size_t capacity, MemoryPool& memory_pool, Hasher hasher,
                                                 EqualityComparator equality_comparator) {
  return HashSetWithArenaAllocator<T, Hasher, EqualityComparator>(capacity, hasher, equality_comparator,
                                                                  ArenaAllocator<T>(memory_pool));
}

template <typename Key, typename Value>
inline HashMap<Key, Value> createHashMap() {
  return createHashMap<Key, Value>(10);
}

template <typename Key, typename Value>
inline HashMap<Key, Value> createHashMap(size_t capacity) {
  return HashMap<Key, Value>(capacity, std::hash<Key>());
}

template <typename Key, typename Value>
inline HashMapWithArenaAllocator<Key, Value> createHashMapWithArenaAllocator(std::size_t capacity,
                                                                             MemoryPool& memory_pool) {
  return createHashMapWithArenaAllocatorAndCustomFunctors<Key, Value>(capacity, memory_pool, std::hash<Key>(),
                                                                      std::equal_to<Key>());
}

template <typename Key, typename Value, typename Hasher, typename EqualityComparator>
inline HashMapWithArenaAllocator<Key, Value, Hasher, EqualityComparator>
createHashMapWithArenaAllocatorAndCustomFunctors(size_t capacity, MemoryPool& memory_pool, Hasher hasher,
                                                 EqualityComparator equality_comparator) {
  return HashMapWithArenaAllocator<Key, Value, Hasher, EqualityComparator>(
      capacity, hasher, equality_comparator, ArenaAllocator<std::pair<const Key, Value>>(memory_pool));
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_HASH_HELPERS_DEFN_H
