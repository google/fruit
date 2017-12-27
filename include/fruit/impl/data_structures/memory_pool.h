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

#ifndef FRUIT_MEMORY_POOL_H
#define FRUIT_MEMORY_POOL_H

#include <vector>

namespace fruit {
namespace impl {

/**
 * A pool of memory that never shrinks and is only deallocated on destruction.
 * See also ArenaAllocator, an Allocator backed by a MemoryPool object.
 */
class MemoryPool {
private:
  // 4KB - 64B.
  // We don't use the full 4KB because malloc also needs to store some metadata for each block, and we want
  // malloc to request <=4KB from the OS.
  constexpr static const std::size_t CHUNK_SIZE = 4 * 1024 - 64;

  std::vector<void*> allocated_chunks;
  // The memory block [first_free, first_free + capacity) is available for allocation
  char* first_free;
  std::size_t capacity;

  void destroy();

public:
  MemoryPool();

  MemoryPool(const MemoryPool&) = delete;
  MemoryPool(MemoryPool&&);
  MemoryPool& operator=(const MemoryPool&) = delete;
  MemoryPool& operator=(MemoryPool&&);
  ~MemoryPool();

  /**
   * Returns a chunk of memory that can hold n T objects.
   * Note that this does *not* construct any T objects at that location.
   */
  template <typename T>
  T* allocate(std::size_t n);
};

} // namespace impl
} // namespace fruit

#include <fruit/impl/data_structures/memory_pool.defn.h>

#endif // FRUIT_MEMORY_POOL_H
