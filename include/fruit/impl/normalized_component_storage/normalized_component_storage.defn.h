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

#ifndef FRUIT_NORMALIZED_COMPONENT_STORAGE_DEFN_H
#define FRUIT_NORMALIZED_COMPONENT_STORAGE_DEFN_H

#include <fruit/impl/normalized_component_storage/normalized_component_storage.h>

namespace fruit {
namespace impl {

inline NormalizedComponentStorage::LazyComponentWithNoArgsSet
NormalizedComponentStorage::createLazyComponentWithNoArgsSet(size_t capacity, MemoryPool& memory_pool) {
  return createHashSetWithArenaAllocatorAndCustomFunctors<LazyComponentWithNoArgs>(
      capacity, memory_pool, NormalizedComponentStorage::HashLazyComponentWithNoArgs(),
      std::equal_to<LazyComponentWithNoArgs>());
}

inline NormalizedComponentStorage::LazyComponentWithArgsSet
NormalizedComponentStorage::createLazyComponentWithArgsSet(size_t capacity, MemoryPool& memory_pool) {
  return createHashSetWithArenaAllocatorAndCustomFunctors<LazyComponentWithArgs>(
      capacity, memory_pool, NormalizedComponentStorage::HashLazyComponentWithArgs(),
      NormalizedComponentStorage::LazyComponentWithArgsEqualTo());
}

inline NormalizedComponentStorage::LazyComponentWithNoArgsReplacementMap
NormalizedComponentStorage::createLazyComponentWithNoArgsReplacementMap(size_t capacity, MemoryPool& memory_pool) {
  return createHashMapWithArenaAllocatorAndCustomFunctors<LazyComponentWithNoArgs, ComponentStorageEntry>(
      capacity, memory_pool, NormalizedComponentStorage::HashLazyComponentWithNoArgs(),
      std::equal_to<LazyComponentWithNoArgs>());
}

inline NormalizedComponentStorage::LazyComponentWithArgsReplacementMap
NormalizedComponentStorage::createLazyComponentWithArgsReplacementMap(size_t capacity, MemoryPool& memory_pool) {
  return createHashMapWithArenaAllocatorAndCustomFunctors<LazyComponentWithArgs, ComponentStorageEntry>(
      capacity, memory_pool, NormalizedComponentStorage::HashLazyComponentWithArgs(),
      NormalizedComponentStorage::LazyComponentWithArgsEqualTo());
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_NORMALIZED_COMPONENT_STORAGE_DEFN_H
