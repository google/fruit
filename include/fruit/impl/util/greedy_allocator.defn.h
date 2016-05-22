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

#ifndef FRUIT_GREEDY_ALLOCATOR_DEFN_H
#define FRUIT_GREEDY_ALLOCATOR_DEFN_H

#include <fruit/impl/util/greedy_allocator.h>

namespace fruit {
namespace impl {

template <typename T>
inline GreedyAllocator<T>::GreedyAllocator(GreedyAllocatorStorage& storage)
  : storage(storage) {
}

template <typename T>
template <typename U>
inline GreedyAllocator<T>::GreedyAllocator(GreedyAllocator<U> other)
  : storage(other.storage) {
}

template <typename T>
inline T* GreedyAllocator<T>::allocate(std::size_t n) {
  return storage.allocate<T>(n);
}

template <typename T>
inline void GreedyAllocator<T>::deallocate(T* p, std::size_t n) {
    return storage.template deallocate<T>(p, n);
}

template <typename T>
template <typename U>
inline bool GreedyAllocator<T>::operator==(const GreedyAllocator<U> other) const {
  return &storage == &(other.storage);
}

template <typename T>
template <typename U>
inline bool GreedyAllocator<T>::operator!=(const GreedyAllocator<U> other) const {
  return &storage != &(other.storage);
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_GREEDY_ALLOCATOR_DEFN_H
