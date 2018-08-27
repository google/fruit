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

#ifndef FRUIT_FIXED_SIZE_ALLOCATOR_H
#define FRUIT_FIXED_SIZE_ALLOCATOR_H

#include <fruit/impl/data_structures/fixed_size_vector.h>
#include <fruit/impl/meta/component.h>
#include <fruit/impl/util/type_info.h>

#if FRUIT_EXTRA_DEBUG
#include <unordered_map>
#endif

namespace fruit {
namespace impl {

/**
 * An allocator where the maximum total size is fixed at construction, and all memory is retained until the allocator
 * object itself is destructed.
 */
class FixedSizeAllocator {
public:
  using destroy_t = void (*)(void*);

private:
  // A pointer to the last used byte in the allocated memory chunk starting at storage_begin.
  char* storage_last_used = nullptr;

  // The chunk of memory that will be used for all allocations.
  char* storage_begin = nullptr;

#if FRUIT_EXTRA_DEBUG
  std::unordered_map<TypeId, std::size_t> remaining_types;
#endif

  // This vector contains the destroy operations that have to be performed at destruction, and
  // the pointers that they must be invoked with. Allows destruction in the correct order.
  // These must be called in reverse order.
  FixedSizeVector<std::pair<destroy_t, void*>> on_destruction;

  // Destroys an object previously created using constructObject().
  template <typename C>
  static void destroyObject(void* p);

  // Calls delete on an object previously allocated using new.
  template <typename C>
  static void destroyExternalObject(void* p);

public:
  // Data used to construct an allocator for a fixed set of types.
  class FixedSizeAllocatorData {
  private:
    std::size_t total_size = 0;
    std::size_t num_types_to_destroy = 0;
#if FRUIT_EXTRA_DEBUG
    std::unordered_map<TypeId, std::size_t> types;
#endif

    static std::size_t maximumRequiredSpace(TypeId type);

    friend class FixedSizeAllocator;

  public:
    // Adds 1 `typeId' to the type set. Multiple copies of the same type are allowed.
    // Each call to this method allows 1 constructObject<T>(...) call on the resulting allocator.
    void addType(TypeId typeId);

    // Each call to this method with getTypeId<T>() allows 1 registerExternallyAllocatedType<T>(...) call on the
    // resulting
    // allocator.
    void addExternallyAllocatedType(TypeId typeId);
  };

  // Constructs an empty allocator (no allocations are allowed).
  FixedSizeAllocator() = default;

  // Constructs an allocator for the type set in FixedSizeAllocatorData.
  FixedSizeAllocator(FixedSizeAllocatorData allocator_data);

  FixedSizeAllocator(FixedSizeAllocator&&);
  FixedSizeAllocator& operator=(FixedSizeAllocator&&);

  FixedSizeAllocator(const FixedSizeAllocator&) = delete;
  FixedSizeAllocator& operator=(const FixedSizeAllocator&) = delete;

  // On destruction, all objects allocated with constructObject() and all externally-allocated objects registered with
  // registerExternallyAllocatedObject() are destroyed.
  ~FixedSizeAllocator();

  // Allocates an object of type T, constructing it with the specified arguments. Similar to:
  // new C(args...)
  template <typename AnnotatedT, typename... Args>
  fruit::impl::meta::UnwrapType<
      fruit::impl::meta::Eval<fruit::impl::meta::RemoveAnnotations(fruit::impl::meta::Type<AnnotatedT>)>>*
  constructObject(Args&&... args);

  template <typename T>
  void registerExternallyAllocatedObject(T* p);
};

} // namespace impl
} // namespace fruit

#include <fruit/impl/data_structures/fixed_size_allocator.defn.h>

#endif // FRUIT_FIXED_SIZE_ALLOCATOR_H
