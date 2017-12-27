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

#ifndef FRUIT_PARTIAL_COMPONENT_STORAGE_H
#define FRUIT_PARTIAL_COMPONENT_STORAGE_H

#include <cstddef>

namespace fruit {
namespace impl {

/**
 * This class stores the data in a PartialComponent<Bindings...>.
 * Instead of dynamically-allocating space for the elements (and then having to move the storage from each
 * PartialComponent class to the next) we only store a reference to the previous PartialComponent's storage
 * and the data needed for the binding (if any).
 * We rely on the fact that the previous PartialComponent objects will only be destroyed after the current
 * one.
 */
template <typename... Bindings>
class PartialComponentStorage; /* {
All specializations support the following methods:

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries);
  std::size_t numBindings();
};*/

template <typename... Bindings>
class PartialComponentStorage {};

} // namespace impl
} // namespace fruit

#include <fruit/impl/component_storage/partial_component_storage.defn.h>

#endif // FRUIT_PARTIAL_COMPONENT_STORAGE_H
