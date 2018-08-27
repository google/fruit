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

#define IN_FRUIT_CPP_FILE 1

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
  delete[] storage_begin;
}

} // namespace impl
} // namespace fruit
