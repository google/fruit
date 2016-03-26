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

#define IN_FRUIT_CPP_FILE

#include <cstdlib>
#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>
#include <fruit/impl/util/type_info.h>

#include <fruit/impl/storage/normalized_component_storage.h>
#include <fruit/impl/storage/component_storage.h>

#include <fruit/impl/data_structures/semistatic_map.templates.h>
#include <fruit/impl/data_structures/semistatic_graph.templates.h>

using std::cout;
using std::endl;

using namespace fruit;
using namespace fruit::impl;

namespace fruit {
namespace impl {

NormalizedComponentStorage::NormalizedComponentStorage(ComponentStorage&& component, const std::vector<TypeId>& exposed_types) {
  std::vector<std::pair<TypeId, BindingData>> bindings_vector(std::move(component.bindings));
  InjectorStorage::normalizeBindings(bindings_vector,
                                     fixed_size_allocator_data,
                                     static_cast<std::vector<CompressedBinding>>(std::move(component.compressed_bindings)),
                                     component.multibindings,
                                     exposed_types,
                                     bindingCompressionInfoMap);
  
  bindings = SemistaticGraph<TypeId, NormalizedBindingData>(InjectorStorage::BindingDataNodeIter{bindings_vector.begin()},
                                                            InjectorStorage::BindingDataNodeIter{bindings_vector.end()});
  
  InjectorStorage::addMultibindings(multibindings, fixed_size_allocator_data, std::move(component.multibindings));
}

} // namespace impl
} // namespace fruit
