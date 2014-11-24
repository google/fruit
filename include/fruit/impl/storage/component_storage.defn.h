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

#ifndef FRUIT_COMPONENT_STORAGE_DEFN_H
#define FRUIT_COMPONENT_STORAGE_DEFN_H

#include "injector_storage.h"
#include "../util/demangle_type_name.h"
#include "../util/type_info.h"
#include "../fruit_assert.h"
#include "../metaprogramming/list.h"
#include "../util/lambda_invoker.h"
#include "../../component.h"

// Not necessary, just to make KDevelop happy.
#include "component_storage.h"

namespace fruit {
namespace impl {

inline void ComponentStorage::addBindingData(std::tuple<TypeId, BindingData> t) {
  if (typeRegistryArray_numUsed < max_num_immediate_bindings) {
    typeRegistryArray[typeRegistryArray_numUsed] = std::make_pair(std::get<0>(t), std::get<1>(t));
    ++typeRegistryArray_numUsed;
  } else {
    typeRegistry.emplace_back(std::get<0>(t), std::get<1>(t));
  }
}

inline void ComponentStorage::addCompressedBindingData(std::tuple<TypeId, TypeId, BindingData> t) {
  compressedBindings.push_back(CompressedBinding{std::get<0>(t), std::get<1>(t), std::get<2>(t)});
}

inline void ComponentStorage::addMultibindingData(std::tuple<TypeId, MultibindingData> t) {
  typeRegistryForMultibindings.emplace_back(std::get<0>(t), std::get<1>(t));
  const BindingDeps* deps = std::get<1>(t).deps;
  if (deps != nullptr) {
    for (std::size_t i = 0; i < deps->num_deps; ++i) {
      multibindingDeps.push_back(deps->deps[i]);
    }
  }
}

} // namespace fruit
} // namespace impl


#endif // FRUIT_COMPONENT_STORAGE_DEFN_H
