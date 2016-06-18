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

#ifndef FRUIT_GREEDY_ALLOCATOR_STORAGE_DEFN_H
#define FRUIT_GREEDY_ALLOCATOR_STORAGE_DEFN_H

#include <fruit/impl/util/greedy_allocator_storage.h>
#include <fruit/impl/fruit_assert.h>

#include <cstdint>

namespace fruit {
namespace impl {

inline GreedyAllocatorStorage GreedyAllocatorStorage::create() {
  return GreedyAllocatorStorage();
}

template <typename T>
inline T* GreedyAllocatorStorage::allocate(std::size_t n) {
#ifdef FRUIT_EXTRA_DEBUG
  numAllocatedObjects += n;
#endif

  static const std::size_t block_size = 4 * 1024;
  std::size_t neededMemory = sizeof(T) * n;
  if (neededMemory > block_size) {
    return static_cast<T*>(operator new(neededMemory));
  }
  std::size_t misalignment = ((std::uintptr_t(unallocatedSpaceBegin) - 1) % alignof(T)) + 1;
  std::size_t alignmentCorrection = alignof(T) - misalignment;
  if (alignmentCorrection + neededMemory
      > std::uintptr_t(unallocatedSpaceEnd) - std::uintptr_t(unallocatedSpaceBegin)) {
    // This is to make sure the push_back below won't cause a reallocation.
    reservedBuffers.reserve(reservedBuffers.size() + 1);
    unallocatedSpaceBegin = static_cast<char*>(operator new(block_size));
    unallocatedSpaceEnd = unallocatedSpaceBegin + block_size;  
    reservedBuffers.push_back(unallocatedSpaceBegin);
  }
  FruitAssert(alignmentCorrection + neededMemory
      <= std::uintptr_t(unallocatedSpaceEnd) - std::uintptr_t(unallocatedSpaceBegin));
  unallocatedSpaceBegin += alignmentCorrection;
  void* result = unallocatedSpaceBegin;
  unallocatedSpaceBegin += neededMemory;
  return static_cast<T*>(result);
}

template <typename T>
inline void GreedyAllocatorStorage::deallocate(T* p, std::size_t n) {
  (void)p;
  (void)n;
#ifdef FRUIT_EXTRA_DEBUG
  FruitAssert(numAllocatedObjects >= n);
  numAllocatedObjects -= n;
#endif
}

inline GreedyAllocatorStorage::~GreedyAllocatorStorage() {
  clear();
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_GREEDY_ALLOCATOR_STORAGE_DEFN_H
