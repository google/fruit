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
#include <functional>
#include <vector>
#include <iostream>
#include <algorithm>
#include "fruit/impl/util/demangle_type_name.h"
#include "fruit/impl/util/type_info.h"

#include "fruit/impl/storage/normalized_component_storage.h"
#include "fruit/impl/storage/component_storage.h"

#include "fruit/impl/data_structures/semistatic_map.templates.h"
#include "fruit/impl/data_structures/semistatic_graph.templates.h"

using std::cout;
using std::endl;

using namespace fruit;
using namespace fruit::impl;

namespace fruit {
namespace impl {

NormalizedComponentStorage::NormalizedComponentStorage(ComponentStorage&& component, std::initializer_list<TypeId> exposed_types) {
  init(std::move(component.bindings),
       std::move(component.compressed_bindings),
       std::move(component.multibindings),
         exposed_types);
}

NormalizedComponentStorage::NormalizedComponentStorage(std::vector<std::pair<TypeId, BindingData>>&& bindings_vector,
                                                       std::vector<CompressedBinding>&& compressed_bindings_vector,
                                                       std::vector<std::pair<TypeId, MultibindingData>>&& multibindings_vector,
                                                       std::initializer_list<TypeId> exposed_types) {
  init(std::move(bindings_vector),
       std::move(compressed_bindings_vector),
       std::move(multibindings_vector),
         exposed_types);
}

void NormalizedComponentStorage::init(std::vector<std::pair<TypeId, BindingData>>&& bindings_vector,
                                      std::vector<CompressedBinding>&& compressed_bindings_vector,
                                      std::vector<std::pair<TypeId, MultibindingData>>&& multibindings_vector,
                                      std::initializer_list<TypeId> exposedTypes) {
  
  InjectorStorage::normalizeBindings(bindings_vector,
                                     total_size,
                                     std::move(compressed_bindings_vector),
                                     multibindings_vector,
                                     exposedTypes);
  
  bindings = SemistaticGraph<TypeId, NormalizedBindingData>(BindingDataNodeIter{bindings_vector.begin()},
                                                            BindingDataNodeIter{bindings_vector.end()});
  
  InjectorStorage::addMultibindings(multibindings, total_size, std::move(multibindings_vector));
}

// TODO: This can't be inline (let alone defined as `=default') with GCC 4.8, while it would work anyway with Clang.
// Consider minimizing the testcase and filing a bug.
NormalizedComponentStorage::~NormalizedComponentStorage() {
}

} // namespace impl
} // namespace fruit
