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

#define IN_FRUIT_CPP_FILE

#include <fruit/impl/data_structures/fixed_size_allocator.h>
#include <fruit/impl/data_structures/fixed_size_vector.templates.h>

using namespace fruit::impl;

namespace fruit {
namespace impl {

FixedSizeAllocator::~FixedSizeAllocator() {
  // Destroy all objects in reverse order.
  std::pair<destroy_t, void*>* p = on_destruction.end();
  while (p != on_destruction.begin()) {
    --p;
    p->first(p->second);
  }
  delete [] storage_begin;
}

// This is not inlined to workaround a GCC 4.8 bug.
// TODO: Inline this once GCC 4.8 is no longer supported.
FixedSizeAllocator::FixedSizeAllocator(FixedSizeAllocator&& x)
  : FixedSizeAllocator() {
  std::swap(storage_begin, x.storage_begin);
  std::swap(storage_last_used, x.storage_last_used);
  std::swap(on_destruction, x.on_destruction);
#ifdef FRUIT_EXTRA_DEBUG
  std::swap(remaining_types, x.remaining_types);
#endif
}

// This is not inlined to workaround a GCC 4.8 bug.
// TODO: Inline this once GCC 4.8 is no longer supported.
FixedSizeAllocator& FixedSizeAllocator::operator=(FixedSizeAllocator&& x) {
  std::swap(storage_begin, x.storage_begin);
  std::swap(storage_last_used, x.storage_last_used);
  std::swap(on_destruction, x.on_destruction);
#ifdef FRUIT_EXTRA_DEBUG
  std::swap(remaining_types, x.remaining_types);
#endif
  return *this;
}

} // namespace impl
} // namespace fruit
