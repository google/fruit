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

#include <fruit/impl/util/demangle_type_name.h>
#include <fruit/impl/util/type_info.h>
#include <fruit/impl/fruit_assert.h>
#include <fruit/impl/meta/vector.h>
#include <fruit/impl/util/lambda_invoker.h>
#include <fruit/component.h>

// Not necessary, just to make KDevelop happy.
#include <fruit/impl/storage/component_storage.h>

namespace fruit {
namespace impl {

inline void ComponentStorage::addBinding(std::tuple<TypeId, BindingData> t) {  
  bindings.push_back(std::make_pair(std::get<0>(t), std::get<1>(t)));
}

inline void ComponentStorage::addCompressedBinding(std::tuple<TypeId, TypeId, BindingData> t) {
  compressed_bindings.push_back(CompressedBinding{std::get<0>(t), std::get<1>(t), std::get<2>(t)});
}

inline void ComponentStorage::addMultibinding(std::tuple<TypeId, MultibindingData> t) {
  multibindings.emplace_back(std::get<0>(t), std::get<1>(t));
}

inline std::vector<std::vector<std::pair<TypeId, BindingData>>>& ComponentStorage::getBindingVectorsCache() {
  thread_local static std::vector<std::vector<std::pair<TypeId, BindingData>>> bindingVectorsCache;
  return bindingVectorsCache;
}

inline std::vector<std::vector<CompressedBinding>>& ComponentStorage::getCompressedBindingVectorsCache() {
  thread_local static std::vector<std::vector<CompressedBinding>> compressedBindingVectorsCache;
  return compressedBindingVectorsCache;
}

inline std::size_t& ComponentStorage::getNumComponentStorageInstancesInThread() {
  thread_local static std::size_t n = 0;
  return n;
}

inline ComponentStorage::ComponentStorage() {
  getNumComponentStorageInstancesInThread()++;
  auto& bindingVectorsCache = getBindingVectorsCache();
  if (!bindingVectorsCache.empty()) {
    bindings = std::move(bindingVectorsCache.back());
    bindingVectorsCache.pop_back();
  } else {
    bindings.reserve(100);
  }
  auto& compressedBindingVectorsCache = getCompressedBindingVectorsCache();
  if (!compressedBindingVectorsCache.empty()) {
    compressed_bindings = std::move(compressedBindingVectorsCache.back());
    compressedBindingVectorsCache.pop_back();
  } else {
    compressed_bindings.reserve(100);
  }
}

inline ComponentStorage::ComponentStorage(const ComponentStorage& other)
    : bindings(other.bindings), 
    compressed_bindings(other.compressed_bindings), 
    multibindings(other.multibindings) {
  getNumComponentStorageInstancesInThread()++;
}

inline ComponentStorage::ComponentStorage(ComponentStorage&& other)
    : bindings(std::move(other.bindings)), 
    compressed_bindings(std::move(other.compressed_bindings)), 
    multibindings(std::move(other.multibindings)) {
  getNumComponentStorageInstancesInThread()++;
}

inline ComponentStorage::~ComponentStorage() {
  if (bindings.capacity() > 0) {
    bindings.clear();
    getBindingVectorsCache().emplace_back(std::move(bindings));
  }
  if (compressed_bindings.capacity() > 0) {
    compressed_bindings.clear();
    getCompressedBindingVectorsCache().emplace_back(std::move(compressed_bindings));
  }
  std::size_t& numInstances = getNumComponentStorageInstancesInThread();
  FruitAssert(numInstances > 0);
  numInstances--;
  // When destroying the last ComponentStorage also empty the caches, to avoid keeping
  // allocated memory for the rest of the program (that might well not create any more
  // ComponentStorage objects).
  if (numInstances == 0) {
    getBindingVectorsCache().clear();
    getCompressedBindingVectorsCache().clear();
  }
}

} // namespace fruit
} // namespace impl


#endif // FRUIT_COMPONENT_STORAGE_DEFN_H
