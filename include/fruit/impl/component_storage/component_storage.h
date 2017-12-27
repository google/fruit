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

#ifndef FRUIT_COMPONENT_STORAGE_H
#define FRUIT_COMPONENT_STORAGE_H

#include <fruit/impl/data_structures/fixed_size_vector.h>
#include <fruit/impl/fruit_internal_forward_decls.h>

namespace fruit {
namespace impl {

/**
 * A component where all types have to be explicitly registered, and all checks are at runtime.
 * Used to implement Component<>, don't use directly.
 * This merely stores the ComponentStorageEntry objects. The real processing will be done in NormalizedComponentStorage
 * and InjectorStorage.
 *
 * This class handles the creation of bindings for types of the forms:
 * - shared_ptr<C>, [const] C*, [const] C&, C (where C is an atomic type)
 * - Annotated<Annotation, T> (with T of the above forms)
 * - Injector<T1, ..., Tk> (with T1, ..., Tk of the above forms).
 */
class ComponentStorage {
private:
  // The entries for this component storage (potentially including lazy component), *in reverse order*.
  FixedSizeVector<ComponentStorageEntry> entries;

  void destroy();

public:
  ComponentStorage() = default;
  ComponentStorage(FixedSizeVector<ComponentStorageEntry>&& entries);
  ComponentStorage(const ComponentStorage&);
  ComponentStorage(ComponentStorage&&);

  ~ComponentStorage();

  FixedSizeVector<ComponentStorageEntry> release() &&;

  std::size_t numEntries() const;

  ComponentStorage& operator=(const ComponentStorage&);
  ComponentStorage& operator=(ComponentStorage&&);
};

} // namespace impl
} // namespace fruit

#include <fruit/impl/component_storage/component_storage.defn.h>

#endif // FRUIT_COMPONENT_STORAGE_H
