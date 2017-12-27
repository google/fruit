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

#ifndef FRUIT_FIXED_SIZE_VECTOR_H
#define FRUIT_FIXED_SIZE_VECTOR_H

#include <cstdlib>
#include <memory>

namespace fruit {
namespace impl {

/**
 * Similar to std::vector<T>, but the capacity is fixed at construction time, and no reallocations ever happen.
 * The type T must be trivially copyable.
 */
template <typename T, typename Allocator = std::allocator<T>>
class FixedSizeVector {
private:
  // This is not yet implemented in libstdc++ (the STL implementation) shipped with GCC (checked until version 4.9.1).
  // static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable.");

  // v_end is before v_begin here, because it's the most commonly accessed field.
  T* v_end;
  T* v_begin;

  std::size_t capacity;
  Allocator allocator;

public:
  using iterator = T*;
  using const_iterator = const T*;

  FixedSizeVector(std::size_t capacity = 0, Allocator allocator = Allocator());
  // Creates a vector with the specified size (and equal capacity) initialized with the specified value.
  FixedSizeVector(std::size_t size, const T& value, Allocator allocator = Allocator());
  ~FixedSizeVector();

  // Copy construction is not allowed, you need to specify the capacity in order to construct the copy.
  FixedSizeVector(const FixedSizeVector& other) = delete;
  FixedSizeVector(const FixedSizeVector& other, std::size_t capacity);

  FixedSizeVector(FixedSizeVector&& other);

  FixedSizeVector& operator=(const FixedSizeVector& other) = delete;
  FixedSizeVector& operator=(FixedSizeVector&& other);

  std::size_t size() const;

  T& operator[](std::size_t i);
  const T& operator[](std::size_t i) const;

  // This yields undefined behavior (instead of reallocating) if the vector's capacity is exceeded.
  void push_back(T x);

  void swap(FixedSizeVector& x);

  // Removes all elements, so size() becomes 0 (but maintains the capacity).
  void clear();

  T* data();
  iterator begin();
  iterator end();

  const T* data() const;
  const_iterator begin() const;
  const_iterator end() const;
};

} // namespace impl
} // namespace fruit

#include <fruit/impl/data_structures/fixed_size_vector.defn.h>

#endif // FRUIT_FIXED_SIZE_VECTOR_H
