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

#ifndef FRUIT_SPARSEHASH_HELPERS_H
#define FRUIT_SPARSEHASH_HELPERS_H

#include <fruit/impl/util/greedy_allocator.h>

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

namespace fruit {
namespace impl {

template <typename T>
using HashSet = boost::unordered_set<T, std::hash<T>, std::equal_to<T>, std::allocator<T>>;

template <typename T>
using HashSetWithGreedyAllocator = boost::unordered_set<T, std::hash<T>, std::equal_to<T>, GreedyAllocator<T>>;

template <typename Key, typename Value>
using HashMap = boost::unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>, std::allocator<std::pair<const Key, Value>>>;

template <typename Key, typename Value>
using HashMapWithGreedyAllocator = boost::unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>, GreedyAllocator<std::pair<const Key, Value>>>;

template <typename T>
HashSet<T> createHashSet();

template <typename T>
HashSetWithGreedyAllocator<T> createHashSet(GreedyAllocatorStorage& allocatorStorage);

template <typename T>
HashSet<T> createHashSet(size_t capacity);

template <typename T>
HashSetWithGreedyAllocator<T> createHashSet(size_t capacity, GreedyAllocatorStorage& allocatorStorage);

template <typename Key, typename Value>
HashMap<Key, Value> createHashMap();

template <typename Key, typename Value>
HashMapWithGreedyAllocator<Key, Value> createHashMap(GreedyAllocatorStorage& allocatorStorage);

template <typename Key, typename Value>
HashMap<Key, Value> createHashMap(size_t capacity);

template <typename Key, typename Value>
HashMapWithGreedyAllocator<Key, Value> createHashMap(size_t capacity, GreedyAllocatorStorage& allocatorStorage);

} // namespace impl
} // namespace fruit

#include <fruit/impl/util/sparsehash_helpers.defn.h>

#endif // FRUIT_SPARSEHASH_HELPERS_H
