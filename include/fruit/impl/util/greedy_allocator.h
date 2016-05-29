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

#ifndef FRUIT_GREEDY_ALLOCATOR_H
#define FRUIT_GREEDY_ALLOCATOR_H

#include <fruit/impl/util/greedy_allocator_storage.h>

namespace fruit {
namespace impl {

/**
 * An Allocator for temporaries used during component normalization and injector
 * construction. All objects allocated via this allocator must be destroyed before
 * returning control to non-Fruit code.
 */
template <typename T>
class GreedyAllocator {
private:
  GreedyAllocatorStorage* storage;
  
  template <typename U>
  friend class GreedyAllocator;
  
public:
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using difference_type = std::ptrdiff_t;
  
  template <typename U>
  struct rebind {
    using other = GreedyAllocator<U>;
  };
  
  GreedyAllocator(GreedyAllocatorStorage& storage);
  
  GreedyAllocator(const GreedyAllocator& other) = default;
  GreedyAllocator(GreedyAllocator&& other) = default;
  
  GreedyAllocator& operator=(const GreedyAllocator& other) = default;
  GreedyAllocator& operator=(GreedyAllocator&& other) = default;
  
  template <typename U>
  GreedyAllocator(GreedyAllocator<U> other);
  
  T* allocate(std::size_t n);
  void deallocate(T* p, std::size_t n);
  
  template <class U, class... Args>
  void construct(U* p, Args&&... args);
  
  template <class U>
  void destroy(U* p);
  
  template <typename U>
  bool operator==(const GreedyAllocator<U> other) const;
  
  template <typename U>
  bool operator!=(const GreedyAllocator<U> other) const;
  
  static void clear();
};

} // namespace impl
} // namespace fruit

#include <fruit/impl/util/greedy_allocator.defn.h>

#endif // FRUIT_GREEDY_ALLOCATOR_H
