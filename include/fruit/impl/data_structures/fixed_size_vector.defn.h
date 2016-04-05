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

#include <utility>
#include <cassert>
#include <cstring>

namespace fruit {
namespace impl {

template <typename T>
inline FixedSizeVector<T>::FixedSizeVector(std::size_t capacity) {
  if (capacity == 0) {
    v_begin = 0;
  } else {
    v_begin = reinterpret_cast<T*>(operator new(sizeof(T) * capacity));
  }
  v_end = v_begin;
#ifdef FRUIT_EXTRA_DEBUG
  v_end_of_storage = v_begin + capacity;
#endif
}

template <typename T>
FixedSizeVector<T>::FixedSizeVector(std::size_t size, const T& value)
  : FixedSizeVector(size) {
  for (std::size_t i = 0; i < size; ++i) {
    push_back(value);
  }
}

template <typename T>
inline FixedSizeVector<T>::~FixedSizeVector() {
  clear();
  operator delete(v_begin);
}

template <typename T>
void FixedSizeVector<T>::clear() {
  for (T* p = v_begin; p != v_end; ++p) {
    p->~T();
  }
  v_end = v_begin;
}

template <typename T>
inline FixedSizeVector<T>::FixedSizeVector(const FixedSizeVector& other, std::size_t capacity)
  : FixedSizeVector(capacity) {
  assert(other.size() <= capacity);
  // This is not just an optimization, we also want to make sure that other.capacity (and therefore
  // also this.capacity) is >0, or we'd pass nullptr to memcpy (although with a size of 0).
  if (other.size() != 0) {
    assert(v_begin != nullptr);
    assert(other.v_begin != nullptr);
    std::memcpy(v_begin, other.v_begin, other.size()*sizeof(T));
  }
  v_end = v_begin + other.size();
}

template <typename T>
inline FixedSizeVector<T>::FixedSizeVector(FixedSizeVector&& other)
  : FixedSizeVector() {
  swap(other);
}

template <typename T>
inline FixedSizeVector<T>& FixedSizeVector<T>::operator=(FixedSizeVector&& other) {
  swap(other);
  return *this;
}

template <typename T>
inline std::size_t FixedSizeVector<T>::size() const {
  return end() - begin();
}

template <typename T>
inline T& FixedSizeVector<T>::operator[](std::size_t i) {
  assert(begin() + i < end());
  return begin()[i];
}

template <typename T>
inline const T& FixedSizeVector<T>::operator[](std::size_t i) const {
  assert(begin() + i < end());
  return begin()[i];
}

template <typename T>
inline void FixedSizeVector<T>::swap(FixedSizeVector& x) {
  std::swap(v_end, x.v_end);
  std::swap(v_begin, x.v_begin); 
#ifdef FRUIT_EXTRA_DEBUG
  std::swap(v_end_of_storage, x.v_end_of_storage);
#endif
}

template <typename T>
inline void FixedSizeVector<T>::push_back(T x) {
#ifdef FRUIT_EXTRA_DEBUG
  assert(v_end != v_end_of_storage);
#endif
  new (v_end) T(x);
  ++v_end;
#ifdef FRUIT_EXTRA_DEBUG
  assert(v_end <= v_end_of_storage);
#endif
}

// This method is covered by tests, even though lcov doesn't detect that.
template <typename T>
inline T* FixedSizeVector<T>::data() {
  return v_begin;
}

template <typename T>
inline T* FixedSizeVector<T>::begin() {
  return v_begin;
}

template <typename T>
inline T* FixedSizeVector<T>::end() {
  return v_end;
}

template <typename T>
inline const T* FixedSizeVector<T>::data() const {
  return v_begin;
}

template <typename T>
inline const T* FixedSizeVector<T>::begin() const {
  return v_begin;
}

template <typename T>
inline const T* FixedSizeVector<T>::end() const {
  return v_end;
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_FIXED_SIZE_VECTOR_DEFN_H
