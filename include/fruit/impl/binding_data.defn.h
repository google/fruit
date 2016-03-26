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

#ifndef FRUIT_BINDING_DATA_DEFN_H
#define FRUIT_BINDING_DATA_DEFN_H

#include <fruit/impl/binding_data.h>

#include <fruit/impl/fruit_internal_forward_decls.h>

#include <cassert>

namespace fruit {
namespace impl {

template <typename L>
struct GetBindingDepsHelper;

template <typename... Ts>
struct GetBindingDepsHelper<fruit::impl::meta::Vector<fruit::impl::meta::Type<Ts>...>> {
  inline const BindingDeps* operator()() {
    static const TypeId types[] = {getTypeId<Ts>()...};
    static const BindingDeps deps = {types, sizeof...(Ts)};
    return &deps;
  }
};


template <typename Deps>
inline const BindingDeps* getBindingDeps() {
  return GetBindingDepsHelper<Deps>()();
};

inline BindingData::BindingData(create_t create, const BindingDeps* deps, bool needs_allocation)
: deps(deps), p(reinterpret_cast<void*>(create)), needs_allocation(needs_allocation) {
}
  
inline BindingData::BindingData(object_t object) 
: deps(nullptr), p(reinterpret_cast<void*>(object)), needs_allocation(false) {
}

inline bool BindingData::isCreated() const {
  return deps == nullptr;
}

inline const BindingDeps* BindingData::getDeps() const {
  assert(deps != nullptr);
  return deps;
}

inline BindingData::create_t BindingData::getCreate() const {
  assert(!isCreated());
  return reinterpret_cast<create_t>(p);
}

inline BindingData::object_t BindingData::getObject() const {
  assert(isCreated());
  return reinterpret_cast<object_t>(p);
}

inline bool BindingData::needsAllocation() const {
  return needs_allocation;
}

inline bool BindingData::operator==(const BindingData& other) const {
  return std::tie(deps, p)
      == std::tie(other.deps, other.p);
}

inline NormalizedBindingData::NormalizedBindingData(BindingData binding_data) {
  if (binding_data.isCreated()) {
    *this = NormalizedBindingData{binding_data.getObject()};
  } else {
    *this = NormalizedBindingData{binding_data.getCreate()};
  }
}

inline NormalizedBindingData::NormalizedBindingData(BindingData::create_t create)
: p(reinterpret_cast<void*>(create)) {
}
  
inline NormalizedBindingData::NormalizedBindingData(BindingData::object_t object) 
: p(reinterpret_cast<void*>(object)) {
}

inline BindingData::create_t NormalizedBindingData::getCreate() const {
  return reinterpret_cast<BindingData::create_t>(p);
}

inline BindingData::object_t NormalizedBindingData::getObject() const {
  return reinterpret_cast<BindingData::object_t>(p);
}

inline void NormalizedBindingData::create(InjectorStorage& storage,
                                          SemistaticGraph<TypeId, NormalizedBindingData>::node_iterator node_itr) {
  BindingData::object_t obj = getCreate()(storage, node_itr);
  p = reinterpret_cast<void*>(obj);
}

inline bool NormalizedBindingData::operator==(const NormalizedBindingData& other) const {
  return std::tie(p)
      == std::tie(other.p);
}

inline MultibindingData::MultibindingData(create_t create, const BindingDeps* deps,
                                          get_multibindings_vector_t get_multibindings_vector, bool needs_allocation)
  : create(create), deps(deps), get_multibindings_vector(get_multibindings_vector), needs_allocation(needs_allocation) {
}
  
inline MultibindingData::MultibindingData(object_t object, get_multibindings_vector_t get_multibindings_vector)
  : object(object), get_multibindings_vector(get_multibindings_vector), needs_allocation(false) {
}

inline NormalizedMultibindingData::Elem::Elem(MultibindingData multibinding_data) {
  create = multibinding_data.create;
  object = multibinding_data.object;
}


} // namespace impl
} // namespace fruit


#endif // FRUIT_BINDING_DATA_DEFN_H
