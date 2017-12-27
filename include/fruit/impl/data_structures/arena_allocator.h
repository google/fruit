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

#ifndef FRUIT_ARENA_ALLOCATOR_H
#define FRUIT_ARENA_ALLOCATOR_H

#include <fruit/impl/data_structures/memory_pool.h>

namespace fruit {
namespace impl {

/**
 * An allocator that allocates memory in pages and only de-allocates memory on destruction.
 * This is useful in code that needs many short-lived allocations.
 * Each ArenaAllocator object should only be accessed by a single thread.
 */
template <typename T>
class ArenaAllocator {
private:
  MemoryPool* pool;

  template <class U>
  friend class ArenaAllocator;

  template <class U, class V>
  friend bool operator==(const ArenaAllocator<U>& x, const ArenaAllocator<V>& y);

  template <class U, class V>
  friend bool operator!=(const ArenaAllocator<U>& x, const ArenaAllocator<V>& y);

public:
  using value_type = T;

  template <typename U>
  struct rebind {
    using other = ArenaAllocator<U>;
  };

  /**
   * Constructs an arena allocator using the specified memory pool.
   * The MemoryPool object must outlive all allocators constructed with it and all allocated objects.
   */
  ArenaAllocator(MemoryPool& memory_pool);

  template <typename U>
  ArenaAllocator(const ArenaAllocator<U>&);

  T* allocate(std::size_t n);
  void deallocate(T* p, std::size_t);
};

template <class T, class U>
bool operator==(const ArenaAllocator<T>&, const ArenaAllocator<U>&);

template <class T, class U>
bool operator!=(const ArenaAllocator<T>&, const ArenaAllocator<U>&);

} // namespace impl
} // namespace fruit

#include <fruit/impl/data_structures/arena_allocator.defn.h>

#endif // FRUIT_ARENA_ALLOCATOR_H
