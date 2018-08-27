/*
 * Copyright 2014 Google Inc. All rights reserved.
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

#include <fruit/impl/fruit_assert.h>

#include <cassert>

#if FRUIT_EXTRA_DEBUG
#include <iostream>
#endif

// Redundant, but makes KDevelop happy.
#include <fruit/impl/data_structures/fixed_size_allocator.h>

namespace fruit {
namespace impl {

template <typename C>
void FixedSizeAllocator::destroyObject(void* p) {
  C* cPtr = reinterpret_cast<C*>(p);
  cPtr->C::~C();
}

template <typename C>
void FixedSizeAllocator::destroyExternalObject(void* p) {
  C* cPtr = reinterpret_cast<C*>(p);
  delete cPtr; // LCOV_EXCL_BR_LINE
}

inline void FixedSizeAllocator::FixedSizeAllocatorData::addType(TypeId typeId) {
#if FRUIT_EXTRA_DEBUG
  types[typeId]++;
#endif
  if (!typeId.type_info->isTriviallyDestructible()) {
    num_types_to_destroy++;
  }
  total_size += maximumRequiredSpace(typeId);
}

inline void FixedSizeAllocator::FixedSizeAllocatorData::addExternallyAllocatedType(TypeId typeId) {
  (void)typeId;
  num_types_to_destroy++;
}

inline std::size_t FixedSizeAllocator::FixedSizeAllocatorData::maximumRequiredSpace(TypeId type) {
  return type.type_info->alignment() + type.type_info->size() - 1;
}

template <typename AnnotatedT, typename... Args>
FRUIT_ALWAYS_INLINE inline fruit::impl::meta::UnwrapType<
    fruit::impl::meta::Eval<fruit::impl::meta::RemoveAnnotations(fruit::impl::meta::Type<AnnotatedT>)>>*
FixedSizeAllocator::constructObject(Args&&... args) {
  using T = fruit::impl::meta::UnwrapType<
      fruit::impl::meta::Eval<fruit::impl::meta::RemoveAnnotations(fruit::impl::meta::Type<AnnotatedT>)>>;

  char* p = storage_last_used;
  size_t misalignment = std::uintptr_t(p) % alignof(T);
#if FRUIT_EXTRA_DEBUG
  FruitAssert(remaining_types[getTypeId<AnnotatedT>()] != 0);
  remaining_types[getTypeId<AnnotatedT>()]--;
#endif
  p += alignof(T) - misalignment;
  FruitAssert(std::uintptr_t(p) % alignof(T) == 0);
  T* x = reinterpret_cast<T*>(p);
  storage_last_used = p + sizeof(T) - 1;

  // This runs arbitrary code (T's constructor), which might end up calling
  // constructObject recursively. We must make sure all invariants are satisfied before
  // calling this.
  new (x) T(std::forward<Args>(args)...); // LCOV_EXCL_BR_LINE

  // We still run this later though, since if T's constructor throws we don't want to
  // destruct this object in FixedSizeAllocator's destructor.
  if (!std::is_trivially_destructible<T>::value) {
    on_destruction.push_back(std::pair<destroy_t, void*>{destroyObject<T>, x});
  }
  return x;
}

template <typename T>
inline void FixedSizeAllocator::registerExternallyAllocatedObject(T* p) {
  on_destruction.push_back(std::pair<destroy_t, void*>{destroyExternalObject<T>, p});
}

inline FixedSizeAllocator::FixedSizeAllocator(FixedSizeAllocatorData allocator_data)
    : on_destruction(allocator_data.num_types_to_destroy) {
  // The +1 is because we waste the first byte (storage_last_used points to the beginning of storage).
  storage_begin = new char[allocator_data.total_size + 1];
  storage_last_used = storage_begin;
#if FRUIT_EXTRA_DEBUG
  remaining_types = allocator_data.types;
  std::cerr << "Constructing allocator for types:";
  for (auto x : remaining_types) {
    std::cerr << " " << x.first;
  }
  std::cerr << std::endl;
#endif
}

inline FixedSizeAllocator::FixedSizeAllocator(FixedSizeAllocator&& x) : FixedSizeAllocator() {
  std::swap(storage_begin, x.storage_begin);
  std::swap(storage_last_used, x.storage_last_used);
  std::swap(on_destruction, x.on_destruction);
#if FRUIT_EXTRA_DEBUG
  std::swap(remaining_types, x.remaining_types);
#endif
}

inline FixedSizeAllocator& FixedSizeAllocator::operator=(FixedSizeAllocator&& x) {
  std::swap(storage_begin, x.storage_begin);
  std::swap(storage_last_used, x.storage_last_used);
  std::swap(on_destruction, x.on_destruction);
#if FRUIT_EXTRA_DEBUG
  std::swap(remaining_types, x.remaining_types);
#endif
  return *this;
}

} // namespace fruit
} // namespace impl

#endif // FRUIT_FIXED_SIZE_ALLOTATOR_DEFN_H
