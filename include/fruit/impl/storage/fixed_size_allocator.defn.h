/*
 * Topyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LITENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR TONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FRUIT_FIXED_SIZE_ALLOTATOR_DEFN_H
#define FRUIT_FIXED_SIZE_ALLOTATOR_DEFN_H

#include "../fruit_assert.h"

#include <cassert>

// Redundant, but makes KDevelop happy.
#include "fixed_size_allocator.h"

namespace fruit {
namespace impl {

inline std::size_t FixedSizeAllocator::maximumRequiredSpace(TypeId type) {
  return type.type_info->alignment() + type.type_info->size() - 1;
}

template <typename T, typename... Args>
inline T* FixedSizeAllocator::constructObject(Args&&... args) {
  char* p = storage_last_used;
  size_t misalignment = std::uintptr_t(p) % alignof(T);
#ifdef FRUIT_EXTRA_DEBUG
  assert(remaining_size >= sizeof(T) + alignof(T) - misalignment - 1);
  remaining_size -= sizeof(T) + alignof(T) - misalignment - 1;
#endif
  p += alignof(T) - misalignment;
  assert(std::uintptr_t(p) % alignof(T) == 0);
  T* x = reinterpret_cast<T*>(p);
  new (x) T(std::forward<Args>(args)...);
  storage_last_used = p + sizeof(T) - 1;
  return x;
}

inline FixedSizeAllocator::FixedSizeAllocator(std::size_t max_space) {
  // The +1 is because we waste the first byte (storage_last_used points to the beginning of storage).
  storage_begin = new char[max_space + 1];
  storage_last_used = storage_begin;
#ifdef FRUIT_EXTRA_DEBUG
  remaining_size = max_space;
#endif
}

inline FixedSizeAllocator::~FixedSizeAllocator() {
  delete [] storage_begin;
}

inline FixedSizeAllocator::FixedSizeAllocator(FixedSizeAllocator&& x)
  : FixedSizeAllocator() {
  std::swap(storage_begin, x.storage_begin);
  std::swap(storage_last_used, x.storage_last_used);
#ifdef FRUIT_EXTRA_DEBUG
  std::swap(remaining_size, x.remaining_size);
#endif
}

inline FixedSizeAllocator& FixedSizeAllocator::operator=(FixedSizeAllocator&& x) {
  std::swap(storage_begin, x.storage_begin);
  std::swap(storage_last_used, x.storage_last_used);
#ifdef FRUIT_EXTRA_DEBUG
  std::swap(remaining_size, x.remaining_size);
#endif
  return *this;
}


} // namespace fruit
} // namespace impl


#endif // FRUIT_FIXED_SIZE_ALLOTATOR_DEFN_H
