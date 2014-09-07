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

#ifndef FRUIT_NORMALIZED_COMPONENT_STORAGE_INLINES_H
#define FRUIT_NORMALIZED_COMPONENT_STORAGE_INLINES_H

// Redundant, but makes KDevelop happy.
#include "normalized_component_storage.h"

namespace fruit {
namespace impl {

inline NormalizedComponentStorage::BindingData::BindingData(create_t create, createArgument_t createArgument)
: p1(reinterpret_cast<void*>(create)), p2(reinterpret_cast<void*>(createArgument)) {
}
  
inline NormalizedComponentStorage::BindingData::BindingData(destroy_t destroy, object_t object) 
: p1(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(destroy) ^ 1)), p2(reinterpret_cast<void*>(object)) {
}

inline bool NormalizedComponentStorage::BindingData::isCreated() const {
  return reinterpret_cast<uintptr_t>(p1) & 1;
}

inline NormalizedComponentStorage::BindingData::create_t NormalizedComponentStorage::BindingData::getCreate() const {
  return reinterpret_cast<create_t>(p1);
}

inline NormalizedComponentStorage::BindingData::createArgument_t NormalizedComponentStorage::BindingData::getCreateArgument() const {
  return reinterpret_cast<createArgument_t>(p2);
}

inline NormalizedComponentStorage::BindingData::destroy_t NormalizedComponentStorage::BindingData::getDestroy() const {
  return reinterpret_cast<destroy_t>(reinterpret_cast<uintptr_t>(p1) ^ 1);
}

inline NormalizedComponentStorage::BindingData::object_t NormalizedComponentStorage::BindingData::getStoredSingleton() const {
  return reinterpret_cast<object_t>(p2);
}

inline void NormalizedComponentStorage::BindingData::create(InjectorStorage& storage) {
  destroy_t destroyOp;
  object_t obj;
  std::tie(obj, destroyOp) = getCreate()(storage, getCreateArgument());
  (*this) = BindingData(destroyOp, obj);
}

inline bool NormalizedComponentStorage::BindingData::operator==(const BindingData& other) const {
  return std::tie(p1, p2)
      == std::tie(other.p1, other.p2);
}

inline bool NormalizedComponentStorage::BindingData::operator<(const BindingData& other) const {
  // `destroy' is intentionally not compared.
  // If the others are equal it should also be equal. If it isn't, the two BindingData structs
  // are still equivalent because they produce the same injected object.
  return std::tie(p1, p2)
       < std::tie(other.p1, other.p2);
}

inline NormalizedComponentStorage::NormalizedComponentStorage() {
#ifndef FRUIT_NO_SPARSE_HASH
  typeRegistry.set_empty_key(nullptr);
  typeRegistryForMultibindings.set_empty_key(nullptr);
#endif
}

inline NormalizedComponentStorage::NormalizedComponentStorage(NormalizedComponentStorage&& other)
  : NormalizedComponentStorage() {
  swap(other);
}

inline NormalizedComponentStorage& NormalizedComponentStorage::operator=(NormalizedComponentStorage&& other) {
  swap(other);
  return *this;
}

inline NormalizedComponentStorage& NormalizedComponentStorage::operator=(const NormalizedComponentStorage& other) {
  NormalizedComponentStorage tmp(other);
  swap(tmp);
  return *this;
}

inline void NormalizedComponentStorage::swap(NormalizedComponentStorage& other) {
  std::swap(total_size, other.total_size);
  typeRegistry.swap(other.typeRegistry);
  typeRegistryForMultibindings.swap(other.typeRegistryForMultibindings);
}

} // namespace impl
} // namespace fruit


#endif // FRUIT_NORMALIZED_COMPONENT_STORAGE_INLINES_H
