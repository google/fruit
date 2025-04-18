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

#ifndef FRUIT_NORMALIZED_COMPONENT_STORAGE_HOLDER_H
#define FRUIT_NORMALIZED_COMPONENT_STORAGE_HOLDER_H

#include <fruit/fruit_forward_decls.h>
#include <fruit/impl/data_structures/arena_allocator.h>
#include <fruit/impl/data_structures/memory_pool.h>
#include <fruit/impl/fruit_internal_forward_decls.h>

namespace fruit {
namespace impl {

/**
 * A wrapper around NormalizedComponentStorage, holding the NormalizedComponentStorage
 * through a unique_ptr so that we don't need to include NormalizedComponentStorage in
 * fruit.h.
 */
class NormalizedComponentStorageHolder {
private:
  // This is semantically a std::unique_ptr, but we can't use std::unique_ptr here in C++23
  // because it would try to instantiate std::unique_ptr<NormalizedComponentStorage>'s destructor
  // and that requires including the definition of NormalizedComponentStorage (that we don't
  // want to include from here / fruit.h).
  NormalizedComponentStorage* storage;

  friend class InjectorStorage;

  template <typename... P>
  friend class fruit::Injector;

public:
  // These are just used as tags to select the desired constructor.
  struct WithUndoableCompression {};
  struct WithPermanentCompression {};

  NormalizedComponentStorageHolder() noexcept = default;

  /**
   * The MemoryPool is only used during construction, the constructed object *can* outlive the memory pool.
   */
  NormalizedComponentStorageHolder(ComponentStorage&& component,
                                   const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types,
                                   MemoryPool& memory_pool, WithUndoableCompression);

  NormalizedComponentStorageHolder(NormalizedComponentStorageHolder&& other) noexcept
      : storage(other.storage) {
      other.storage = nullptr;
  }
  NormalizedComponentStorageHolder(const NormalizedComponentStorageHolder&) = delete;

  NormalizedComponentStorageHolder& operator=(NormalizedComponentStorageHolder&&) = delete;
  NormalizedComponentStorageHolder& operator=(const NormalizedComponentStorageHolder&) = delete;

  ~NormalizedComponentStorageHolder() noexcept;
};

} // namespace impl
} // namespace fruit

#endif // FRUIT_NORMALIZED_COMPONENT_STORAGE_HOLDER_H
