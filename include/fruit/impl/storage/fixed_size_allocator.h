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

#ifndef FRUIT_FIXED_SIZE_ALLOCATOR_H
#define FRUIT_FIXED_SIZE_ALLOCATOR_H

#include "../util/type_info.h"

#include <vector>

namespace fruit {
namespace impl {

/**
 * An allocator where the maximum total size is fixed at construction, and all memory is retained until the allocator object itself is destructed.
 */
class FixedSizeAllocator {
private:
  // A pointer to the last used byte in the allocated memory chunk starting at storage_begin.
  char* storage_last_used = nullptr;
  
  // The chunk of memory that will be used for all allocations.
  char* storage_begin = nullptr;
  
#ifdef FRUIT_EXTRA_DEBUG
  std::size_t remaining_size = 0;
#endif
  
public:
  // Constructs an empty allocator (no allocations are allowed).
  FixedSizeAllocator() = default;
  
  // Constructs an allocator with size max_space.
  // max_space must be (at least) the sum of maximumRequiredSpace(getTypeId<T>()) over all allocated objects.
  // Note that the sum of sizeof(T) is not enough.
  FixedSizeAllocator(std::size_t max_space);
  
  FixedSizeAllocator(FixedSizeAllocator&&);
  FixedSizeAllocator& operator=(FixedSizeAllocator&&);
  
  FixedSizeAllocator(const FixedSizeAllocator&) = delete;
  FixedSizeAllocator& operator=(const FixedSizeAllocator&) = delete;
  
  ~FixedSizeAllocator();
  
  static std::size_t maximumRequiredSpace(TypeId type);
  
  // Allocates an object of type T, constructing it with the specified arguments. Similar to:
  // new C(args...)
  template <typename T, typename... Args>
  T* constructObject(Args&&... args);
};

} // namespace impl
} // namespace fruit

#include "fixed_size_allocator.defn.h"


#endif // FRUIT_FIXED_SIZE_ALLOCATOR_H
