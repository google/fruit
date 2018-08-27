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

#ifndef FRUIT_MEMORY_POOL_DEFN_H
#define FRUIT_MEMORY_POOL_DEFN_H

#include <fruit/impl/data_structures/memory_pool.h>
#include <fruit/impl/fruit-config.h>
#include <fruit/impl/fruit_assert.h>

#include <cstdint>

namespace fruit {
namespace impl {

inline MemoryPool::MemoryPool() : first_free(nullptr), capacity(0) {}

inline MemoryPool::MemoryPool(MemoryPool&& other)
    : allocated_chunks(std::move(other.allocated_chunks)), first_free(other.first_free), capacity(other.capacity) {
  // This is to be sure that we don't double-deallocate.
  other.allocated_chunks.clear();
}

inline MemoryPool& MemoryPool::operator=(MemoryPool&& other) {
  destroy();

  allocated_chunks = std::move(other.allocated_chunks);
  first_free = other.first_free;
  capacity = other.capacity;

  // This is to be sure that we don't double-deallocate.
  other.allocated_chunks.clear();

  return *this;
}

inline MemoryPool::~MemoryPool() {
  destroy();
}

template <typename T>
FRUIT_ALWAYS_INLINE inline T* MemoryPool::allocate(std::size_t n) {
#if FRUIT_DISABLE_ARENA_ALLOCATION
  void* p = operator new(n * sizeof(T));
  allocated_chunks.push_back(p);
  return static_cast<T*>(p);
#else

  if (n == 0) {
    n = 1;
  }
  std::size_t misalignment = std::uintptr_t(first_free) % alignof(T);
  std::size_t padding = alignof(T) - (sizeof(T) % alignof(T));
  std::size_t required_space = n * (sizeof(T) + padding);
  std::size_t required_space_in_chunk = required_space + (alignof(T) - misalignment);
  if (required_space_in_chunk > capacity) {
    // This is to make sure that the push_back below won't throw.
    if (allocated_chunks.size() == allocated_chunks.capacity()) {
      allocated_chunks.reserve(1 + 2 * allocated_chunks.size());
    }
    void* p;
    if (required_space > CHUNK_SIZE) {
      p = operator new(required_space); // LCOV_EXCL_BR_LINE
    } else {
      p = operator new(CHUNK_SIZE);
      first_free = static_cast<char*>(p) + required_space;
      capacity = CHUNK_SIZE - required_space;
    }
    allocated_chunks.push_back(p);
    return static_cast<T*>(p);
  } else {
    FruitAssert(first_free != nullptr);
    void* p = first_free + misalignment;
    first_free += required_space_in_chunk;
    capacity -= required_space_in_chunk;
    return static_cast<T*>(p);
  }
#endif
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_MEMORY_POOL_DEFN_H
