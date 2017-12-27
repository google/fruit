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

#ifndef FRUIT_ARENA_ALLOCATOR_DEFN_H
#define FRUIT_ARENA_ALLOCATOR_DEFN_H

#include <fruit/impl/data_structures/arena_allocator.h>

namespace fruit {
namespace impl {

template <typename T>
inline ArenaAllocator<T>::ArenaAllocator(MemoryPool& memory_pool) : pool(&memory_pool) {}

template <typename T>
template <typename U>
inline ArenaAllocator<T>::ArenaAllocator(const ArenaAllocator<U>& other) : pool(other.pool) {}

template <typename T>
inline T* ArenaAllocator<T>::allocate(std::size_t n) {
  return pool->allocate<T>(n);
}

template <typename T>
void ArenaAllocator<T>::deallocate(T*, std::size_t) {
  // Intentionally empty, the memory will be deallocated when the MemoryPool is destroyed.
}

template <class T, class U>
bool operator==(const ArenaAllocator<T>& x, const ArenaAllocator<U>& y) {
  return x.pool == y.pool;
}

template <class T, class U>
bool operator!=(const ArenaAllocator<T>& x, const ArenaAllocator<U>& y) {
  return x.pool != y.pool;
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_ARENA_ALLOCATOR_DEFN_H
