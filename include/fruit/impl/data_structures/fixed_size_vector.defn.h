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
FixedSizeVector<T>::~FixedSizeVector() {
  for (T* p = begin(); p != end(); ++p) {
    p->~T();
  }
  operator delete(v_begin);
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

template <typename T>
inline T* FixedSizeVector<T>::begin() {
  return v_begin;
}

template <typename T>
inline T* FixedSizeVector<T>::end() {
  return v_end;
}

} // namespace impl
} // namespace fruit

#include "fixed_size_vector.defn.h"

#endif // FRUIT_FIXED_SIZE_VECTOR_DEFN_H
