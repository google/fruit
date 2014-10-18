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

#ifndef FRUIT_NORMALIZED_COMPONENT_STORAGE_H
#define FRUIT_NORMALIZED_COMPONENT_STORAGE_H

#include "type_info.h"
#include "binding_data.h"
#include "semistatic_map.h"
#include "../fruit_forward_decls.h"

#include <memory>
#include <unordered_map>

namespace fruit {
  
namespace impl {

/**
 * Similar to ComponentStorage, but used a normalized representation to minimize the amount
 * of work needed to turn this into an injector. However, adding bindings to a normalized
 * component is slower than adding them to a simple component.
 */
class NormalizedComponentStorage {
public:  
  using BindingVectors = std::pair<std::vector<BindingData>,
                                   std::vector<std::pair<const TypeInfo*, BindingDataForMultibinding>>>;
  
private:
  // Maps the type index of a type T to the corresponding singleton object or create operation.
  SemistaticMap typeRegistry;
  
  // Maps the type index of a type T to a set of the corresponding BindingData objects (for multibindings).
  std::unordered_map<const TypeInfo*, BindingDataVectorForMultibinding> typeRegistryForMultibindings;
  
  // The sum of (typeInfo->alignment() + typeInfo->size() - 1) for every binding and multibinding.
  // A new[total_size] allocates enough memory to construct all types registered in this component.
  size_t total_size = 0;
  
  friend class InjectorStorage;
  
public:
  static fruit::impl::NormalizedComponentStorage mergeComponentStorages(fruit::impl::NormalizedComponentStorage&& normalizedStorage,
                                                                        fruit::impl::ComponentStorage&& storage);
  
  NormalizedComponentStorage();
  
  NormalizedComponentStorage(NormalizedComponentStorage&&);
  NormalizedComponentStorage(const NormalizedComponentStorage&) = default;
  
  NormalizedComponentStorage& operator=(NormalizedComponentStorage&&);
  NormalizedComponentStorage& operator=(const NormalizedComponentStorage&);
  
  void swap(NormalizedComponentStorage& other);
  
  NormalizedComponentStorage(BindingVectors&& bindingVectors);
  
  ~NormalizedComponentStorage();
};

} // namespace impl
} // namespace fruit

#include "normalized_component_storage.inlines.h"

#endif // FRUIT_NORMALIZED_COMPONENT_STORAGE_H
