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

#include <sparsehash/dense_hash_set>
#include <sparsehash/dense_hash_map>

namespace fruit {
namespace impl {

template <typename T>
using HashSet = google::dense_hash_set<T, std::hash<T>>;

template <typename Key, typename Value>
using HashMap = google::dense_hash_map<Key, Value, std::hash<Key>>;

template <typename T>
HashSet<T> createHashSet(T invalidValue1, T invalidValue2);

template <typename T>
HashSet<T> createHashSet(size_t capacity, T invalidValue1, T invalidValue2);

template <typename Key, typename Value>
HashMap<Key, Value> createHashMap(Key invalidKey1, Key invalidKey2);

template <typename Key, typename Value>
HashMap<Key, Value> createHashMap(size_t capacity, Key invalidKey1, Key invalidKey2);

} // namespace impl
} // namespace fruit

#include <fruit/impl/util/sparsehash_helpers.defn.h>

#endif // FRUIT_SPARSEHASH_HELPERS_H
