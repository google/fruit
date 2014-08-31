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

#ifndef FRUIT_INJECTOR_STORAGE_INLINES_H
#define FRUIT_INJECTOR_STORAGE_INLINES_H

// Redundant, but makes KDevelop happy.
#include "injector_storage.h"

namespace fruit {
namespace impl {

inline InjectorStorage::BindingData::BindingData(create_t create, createArgument_t createArgument)
: p1(reinterpret_cast<void*>(create)), p2(reinterpret_cast<void*>(createArgument)) {
}
  
inline InjectorStorage::BindingData::BindingData(destroy_t destroy, object_t object) 
: p1(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(destroy) ^ 1)), p2(reinterpret_cast<void*>(object)) {
}

inline bool InjectorStorage::BindingData::isCreated() const {
  return reinterpret_cast<uintptr_t>(p1) & 1;
}

inline InjectorStorage::BindingData::create_t InjectorStorage::BindingData::getCreate() const {
  return reinterpret_cast<create_t>(p1);
}

inline InjectorStorage::BindingData::createArgument_t InjectorStorage::BindingData::getCreateArgument() const {
  return reinterpret_cast<createArgument_t>(p2);
}

inline InjectorStorage::BindingData::destroy_t InjectorStorage::BindingData::getDestroy() const {
  return reinterpret_cast<destroy_t>(reinterpret_cast<uintptr_t>(p1) ^ 1);
}

inline InjectorStorage::BindingData::object_t InjectorStorage::BindingData::getStoredSingleton() const {
  return reinterpret_cast<object_t>(p2);
}

inline void InjectorStorage::BindingData::create(InjectorStorage& storage) {
  destroy_t destroyOp;
  object_t obj;
  std::tie(obj, destroyOp) = getCreate()(storage, getCreateArgument());
  (*this) = BindingData(destroyOp, obj);
}

inline bool InjectorStorage::BindingData::operator==(const BindingData& other) const {
  return std::tie(p1, p2)
      == std::tie(other.p1, other.p2);
}

inline bool InjectorStorage::BindingData::operator<(const BindingData& other) const {
  // `destroy' is intentionally not compared.
  // If the others are equal it should also be equal. If it isn't, the two BindingData structs
  // are still equivalent because they produce the same injected object.
  return std::tie(p1, p2)
       < std::tie(other.p1, other.p2);
}

inline void InjectorStorage::check(bool b, const char* message) {
  check(b, [=](){return std::string(message);});
}

} // namespace impl
} // namespace fruit


#endif // FRUIT_INJECTOR_STORAGE_INLINES_H
