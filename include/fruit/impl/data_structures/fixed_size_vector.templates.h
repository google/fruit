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

#ifndef FRUIT_FIXED_SIZE_VECTOR_TEMPLATES_H
#define FRUIT_FIXED_SIZE_VECTOR_TEMPLATES_H

#if !IN_FRUIT_CPP_FILE
#error "Fruit .template.h file included in non-cpp file."
#endif

#include <fruit/impl/data_structures/fixed_size_vector.h>

#include <fruit/impl/fruit_assert.h>

namespace fruit {
namespace impl {

template <typename T, typename Allocator>
FixedSizeVector<T, Allocator>::FixedSizeVector(const FixedSizeVector& other, std::size_t capacity)
    : FixedSizeVector(capacity, other.allocator) {
  FruitAssert(other.size() <= capacity);
  // This is not just an optimization, we also want to make sure that other.capacity (and therefore
  // also this.capacity) is >0, or we'd pass nullptr to memcpy (although with a size of 0).
  if (other.size() != 0) {
    FruitAssert(v_begin != nullptr);
    FruitAssert(other.v_begin != nullptr);
    std::memcpy(v_begin, other.v_begin, other.size() * sizeof(T));
  }
  v_end = v_begin + other.size();
}

template <typename T, typename Allocator>
FixedSizeVector<T, Allocator>::FixedSizeVector(std::size_t size, const T& value, Allocator allocator)
    : FixedSizeVector(size, allocator) {
  for (std::size_t i = 0; i < size; ++i) {
    push_back(value);
  }
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_FIXED_SIZE_VECTOR_TEMPLATES_H
