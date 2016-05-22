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

#ifndef FRUIT_GREEDY_ALLOCATOR_STORAGE_H
#define FRUIT_GREEDY_ALLOCATOR_STORAGE_H

#include <vector>

namespace fruit {
namespace impl {

/**
 * This class contains the logic/data for an allocator that reserves memory in 4KB blocks
 * and never frees memory until it's destroyed.
 * All objects allocated with this allocator must be destroyed before the allocator is
 * destroyed.
 * Note that this is *not* an Allocator class itself, it's meant to be used by
 * GreedyAllocator.
 */
class GreedyAllocatorStorage {
private:
  std::vector<char*> reservedBuffers;
  char* unallocatedSpaceBegin = nullptr;
  char* unallocatedSpaceEnd = nullptr;
  
#ifdef FRUIT_EXTRA_DEBUG
  std::size_t numAllocatedObjects = 0;
#endif
  
  GreedyAllocatorStorage() = default;
  
public:
  template <typename T>
  T* allocate(size_t n);
  
  template <typename T>
  void deallocate(T* p, size_t n);
  
  // This class exposes a create() method instead of making the default constructor public
  // to make it more explicit when we're creating a new allocator storage.
  static GreedyAllocatorStorage create();
  
  GreedyAllocatorStorage(GreedyAllocatorStorage&&) = default;
  GreedyAllocatorStorage(const GreedyAllocatorStorage&) = delete;
  GreedyAllocatorStorage& operator=(GreedyAllocatorStorage&&) = default;
  GreedyAllocatorStorage& operator=(const GreedyAllocatorStorage&) = delete;
  
  void clear();
  
  ~GreedyAllocatorStorage();
};

} // namespace impl
} // namespace fruit

#include <fruit/impl/util/greedy_allocator_storage.defn.h>

#endif // FRUIT_GREEDY_ALLOCATOR_STORAGE_H
