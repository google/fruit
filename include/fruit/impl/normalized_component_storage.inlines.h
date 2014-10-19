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

inline BindingData::BindingData(create_t create)
: is_created(false), x2(reinterpret_cast<Unsigned>(create)) {
}
  
inline BindingData::BindingData(object_t object) 
: is_created(true), x2(reinterpret_cast<Unsigned>(object)) {
}

inline bool BindingData::isCreated() const {
  return is_created;
}

inline BindingData::create_t BindingData::getCreate() const {
  assert(!is_created);
  return reinterpret_cast<create_t>(x2);
}

inline BindingData::object_t BindingData::getStoredSingleton() const {
  assert(is_created);
  return reinterpret_cast<object_t>(x2);
}

inline BindingData::destroy_t BindingData::create(
    InjectorStorage& storage) {
  assert(!is_created);
  destroy_t destroyOp;
  object_t obj;
  std::tie(obj, destroyOp) = getCreate()(storage);
  is_created = true;
  x2 = reinterpret_cast<Unsigned>(obj);
  return destroyOp;
}

inline bool BindingData::operator==(const BindingData& other) const {
  return std::tie(is_created, x2)
      == std::tie(other.is_created, other.x2);
}

inline bool BindingData::operator<(const BindingData& other) const {
  // `destroy' is intentionally not compared.
  // If the others are equal it should also be equal. If it isn't, the two BindingData structs
  // are still equivalent because they produce the same injected object.
  return std::tie(is_created, x2)
       < std::tie(other.is_created, other.x2);
}


} // namespace impl
} // namespace fruit


#endif // FRUIT_NORMALIZED_COMPONENT_STORAGE_INLINES_H
