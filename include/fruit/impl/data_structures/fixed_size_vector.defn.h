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

#ifndef FRUIT_FIXED_SIZE_VECTOR_DEFN_H
#define FRUIT_FIXED_SIZE_VECTOR_DEFN_H

#include <fruit/impl/data_structures/fixed_size_vector.h>

#include <fruit/impl/fruit_assert.h>

#include <cassert>
#include <cstring>
#include <utility>

namespace fruit {
namespace impl {

template <typename T, typename Allocator>
inline FixedSizeVector<T, Allocator>::FixedSizeVector(std::size_t capacity, Allocator allocator)
    : capacity(capacity), allocator(allocator) {
  if (capacity == 0) { // LCOV_EXCL_BR_LINE
    v_begin = 0;
  } else {
    v_begin = allocator.allocate(capacity);
  }
  v_end = v_begin;
}

template <typename T, typename Allocator>
inline FixedSizeVector<T, Allocator>::~FixedSizeVector() {
  clear();
  if (capacity != 0) {
    allocator.deallocate(v_begin, capacity);
  }
}

template <typename T, typename Allocator>
inline FixedSizeVector<T, Allocator>::FixedSizeVector(FixedSizeVector&& other) : FixedSizeVector() {
  swap(other);
}

template <typename T, typename Allocator>
inline FixedSizeVector<T, Allocator>& FixedSizeVector<T, Allocator>::operator=(FixedSizeVector&& other) {
  swap(other);
  return *this;
}

template <typename T, typename Allocator>
inline std::size_t FixedSizeVector<T, Allocator>::size() const {
  return end() - begin();
}

template <typename T, typename Allocator>
inline T& FixedSizeVector<T, Allocator>::operator[](std::size_t i) {
  FruitAssert(begin() + i < end());
  return begin()[i];
}

template <typename T, typename Allocator>
inline const T& FixedSizeVector<T, Allocator>::operator[](std::size_t i) const {
  FruitAssert(begin() + i < end());
  return begin()[i];
}

template <typename T, typename Allocator>
inline void FixedSizeVector<T, Allocator>::swap(FixedSizeVector& x) {
  std::swap(v_end, x.v_end);
  std::swap(v_begin, x.v_begin);
  std::swap(capacity, x.capacity);
}

template <typename T, typename Allocator>
inline void FixedSizeVector<T, Allocator>::push_back(T x) {
#if FRUIT_EXTRA_DEBUG
  FruitAssert(v_end != v_begin + capacity);
#endif
  new (v_end) T(x); // LCOV_EXCL_BR_LINE
  ++v_end;
#if FRUIT_EXTRA_DEBUG
  FruitAssert(v_end <= v_begin + capacity);
#endif
}

// This method is covered by tests, even though lcov doesn't detect that.
template <typename T, typename Allocator>
inline T* FixedSizeVector<T, Allocator>::data() {
  return v_begin;
}

template <typename T, typename Allocator>
inline T* FixedSizeVector<T, Allocator>::begin() {
  return v_begin;
}

template <typename T, typename Allocator>
inline T* FixedSizeVector<T, Allocator>::end() {
  return v_end;
}

template <typename T, typename Allocator>
inline const T* FixedSizeVector<T, Allocator>::data() const {
  return v_begin;
}

template <typename T, typename Allocator>
inline const T* FixedSizeVector<T, Allocator>::begin() const {
  return v_begin;
}

template <typename T, typename Allocator>
inline const T* FixedSizeVector<T, Allocator>::end() const {
  return v_end;
}

template <typename T, typename Allocator>
inline void FixedSizeVector<T, Allocator>::clear() {
  for (T* p = v_begin; p != v_end; ++p) {
    p->~T();
  }
  v_end = v_begin;
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_FIXED_SIZE_VECTOR_DEFN_H
