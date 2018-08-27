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

#ifndef FRUIT_HASH_HELPERS_H
#define FRUIT_HASH_HELPERS_H

#include <fruit/impl/data_structures/arena_allocator.h>
#include <fruit/impl/fruit-config.h>

#if !IN_FRUIT_CPP_FILE
// We don't want to include it in public headers to save some compile time.
#error "hash_helpers included in non-cpp file."
#endif

#if FRUIT_USES_BOOST
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#else
#include <unordered_map>
#include <unordered_set>
#endif

namespace fruit {
namespace impl {

#if FRUIT_USES_BOOST
template <typename T, typename Hasher = std::hash<T>, typename EqualityComparator = std::equal_to<T>>
using HashSet = boost::unordered_set<T, Hasher, EqualityComparator>;

template <typename T, typename Hasher = std::hash<T>, typename EqualityComparator = std::equal_to<T>>
using HashSetWithArenaAllocator = boost::unordered_set<T, Hasher, EqualityComparator, ArenaAllocator<T>>;

template <typename Key, typename Value, typename Hasher = std::hash<Key>>
using HashMap = boost::unordered_map<Key, Value, Hasher>;

template <typename Key, typename Value, typename Hasher = std::hash<Key>,
          typename EqualityComparator = std::equal_to<Key>>
using HashMapWithArenaAllocator =
    boost::unordered_map<Key, Value, Hasher, EqualityComparator, ArenaAllocator<std::pair<const Key, Value>>>;

#else
template <typename T, typename Hasher = std::hash<T>, typename EqualityComparator = std::equal_to<T>>
using HashSet = std::unordered_set<T, Hasher, EqualityComparator>;

template <typename T, typename Hasher = std::hash<T>, typename EqualityComparator = std::equal_to<T>>
using HashSetWithArenaAllocator = std::unordered_set<T, Hasher, EqualityComparator, ArenaAllocator<T>>;

template <typename Key, typename Value, typename Hasher = std::hash<Key>>
using HashMap = std::unordered_map<Key, Value, Hasher>;

template <typename Key, typename Value, typename Hasher = std::hash<Key>,
          typename EqualityComparator = std::equal_to<Key>>
using HashMapWithArenaAllocator =
    std::unordered_map<Key, Value, Hasher, EqualityComparator, ArenaAllocator<std::pair<const Key, Value>>>;

#endif

template <typename T>
HashSet<T> createHashSet();

template <typename T>
HashSet<T> createHashSet(size_t capacity);

template <typename T>
HashSetWithArenaAllocator<T> createHashSetWithArenaAllocator(size_t capacity, MemoryPool& memory_pool);

template <typename T, typename Hasher, typename EqualityComparator>
HashSetWithArenaAllocator<T, Hasher, EqualityComparator>
createHashSetWithArenaAllocatorAndCustomFunctors(size_t capacity, MemoryPool& memory_pool, Hasher, EqualityComparator);

template <typename Key, typename Value>
HashMap<Key, Value> createHashMap();

template <typename Key, typename Value>
HashMap<Key, Value> createHashMap(size_t capacity);

template <typename Key, typename Value>
HashMapWithArenaAllocator<Key, Value> createHashMapWithArenaAllocator(size_t capacity, MemoryPool& memory_pool);

template <typename Key, typename Value, typename Hasher, typename EqualityComparator>
HashMapWithArenaAllocator<Key, Value, Hasher, EqualityComparator>
createHashMapWithArenaAllocatorAndCustomFunctors(size_t capacity, MemoryPool& memory_pool, Hasher, EqualityComparator);

} // namespace impl
} // namespace fruit

#include <fruit/impl/util/hash_helpers.defn.h>

#endif // FRUIT_HASH_HELPERS_H
