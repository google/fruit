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

#ifndef FRUIT_BINDING_DATA_H
#define FRUIT_BINDING_DATA_H

#include "util/type_info.h"
#include "data_structures/semistatic_graph.h"
#include <vector>
#include <memory>

#ifdef FRUIT_EXTRA_DEBUG
#include <iostream>
#include <string>
#endif

namespace fruit {
namespace impl {

class NormalizedComponentStorage;

class InjectorStorage;

class NormalizedBindingData;

struct BindingDeps {
  // A C-style array of deps
  const TypeId* deps;
  
  // The size of the above array.
  std::size_t num_deps;
};

template <typename... Deps>
const BindingDeps* getBindingDeps() {
  static const TypeId types[] = {getTypeId<Deps>()...};
  static const BindingDeps deps = {types, sizeof...(Deps)};
#ifdef FRUIT_EXTRA_DEBUG
  std::cerr << "In getBindingDeps(): deps.num_deps is " << deps.num_deps << ", expected: " << sizeof...(Deps) << std::endl;
#endif
  return &deps;
};

class BindingData {
public:
  using Unsigned = std::uintptr_t;
  
  using object_t = void*;
  using destroy_t = void(*)(void*);
  using create_t = std::tuple<object_t, destroy_t, void*>(*)(InjectorStorage&,
                                                             SemistaticGraph<TypeId, NormalizedBindingData>::edge_iterator);
  
private:
  // `deps' stores the type IDs that this type depends on.
  // If `deps' itself is nullptr, this binding stores an object instead of a create operation.
  const BindingDeps* deps;
  
  // This stores either:
  // 
  // * create, of type create_t if deps!=nullptr
  //   The return type is a tuple (constructedObject, destroyOperation, ptr), where constructedObject!=null and destroyOperation
  //   is nullptr if no destruction is needed. If destruction is needed, it can be done by calling destroyOperation(ptr).
  // 
  // * storedSingleton, of type object_t if deps==nullptr
  //   The stored singleton, a casted T*.
  void* p;
  
public:
  BindingData() = default;
  
  // Binding data for a singleton not already constructed.
  BindingData(create_t create, const BindingDeps* deps);
    
  // Binding data for a singleton already constructed.
  BindingData(object_t object);
  
  bool isCreated() const;
  
  // This assumes !isCreated().
  const BindingDeps* getDeps() const;
  
  // This assumes !isCreated().
  create_t getCreate() const;
  
  // This assumes isCreated().
  object_t getStoredSingleton() const;
  
  bool operator==(const BindingData& other) const;
  
  // Fairly arbitrary lexicographic comparison, needed for std::set.
  bool operator<(const BindingData& other) const;
};

// A CompressedBinding with interfaceId==getTypeId<I>() and classId==getTypeId<C>() means that if:
// * C is not exposed by the component 
// * I is the only node that depends on C
// * There are no multibindings that directly depend on C
// Then bindingData can be used as BindingData for I instead of the one in typeRegistry, and C can be removed.
struct CompressedBinding {
  TypeId interfaceId;
  TypeId classId;
  BindingData bindingData;
};

class NormalizedBindingData {
private:
  // This stores either:
  // 
  // * create, of type create_t if deps!=nullptr
  //   The return type is a tuple (constructedObject, destroyOperation, ptr), where constructedObject!=null and destroyOperation
  //   is nullptr if no destruction is needed. If destruction is needed, it can be done by calling destroyOperation(ptr).
  // 
  // * storedSingleton, of type object_t if deps==nullptr
  //   The stored singleton, a casted T*.
  void* p;
  
public:
  NormalizedBindingData() = default;
  
  explicit NormalizedBindingData(BindingData bindingData);
  
  // Binding data for a singleton not already constructed.
  NormalizedBindingData(BindingData::create_t create);
    
  // Binding data for a singleton already constructed.
  NormalizedBindingData(BindingData::object_t object);
  
  // This assumes that the graph node is NOT terminal (i.e. that there is no object yet).
  BindingData::create_t getCreate() const;
  
  // This assumes that the graph node is terminal (i.e. that there is an object in this BindingData).
  BindingData::object_t getStoredSingleton() const;
  
  // This assumes that the graph node is NOT terminal (i.e. that there is no object yet).
  // After this call, the graph node must be changed to terminal. Returns the destroy operation (or nullptr if not needed) and the
  // pointer that the destroy operation must be invoked on.
  std::pair<BindingData::destroy_t, void*> create(InjectorStorage& storage, 
                                                  typename SemistaticGraph<TypeId, NormalizedBindingData>::edge_iterator
                                                      depsBegin);
  
  bool operator==(const NormalizedBindingData& other) const;
  
  // Fairly arbitrary lexicographic comparison, needed for std::set.
  bool operator<(const NormalizedBindingData& other) const;
};

struct MultibindingData {
  using object_t = void*;
  using destroy_t = void(*)(void*);
  using create_t = std::pair<object_t, destroy_t>(*)(InjectorStorage&);
  
  // This is nullptr if the object is already constructed.
  create_t create = nullptr;
  
  // This is nullptr if the object hasn't been constructed yet.
  object_t object = nullptr;
  
  // This is nullptr if no destruction is needed, or if the object hasn't been constructed yet.
  destroy_t destroy = nullptr;
  
  // Returns the std::vector<T*> of instances, or nullptr if none.
  // Caches the result in the `v' member of NormalizedMultibindingData.
  std::shared_ptr<char>(*getSingletonsVector)(InjectorStorage&);
};

struct NormalizedMultibindingData {
  
  struct Elem {
    explicit Elem(MultibindingData multibindingData) {
      create = multibindingData.create;
      object = multibindingData.object;
      destroy = multibindingData.destroy;
    }
    
    // This is nullptr if the object is already constructed.
    MultibindingData::create_t create = nullptr;
    
    // This is nullptr if the object hasn't been constructed yet.
    MultibindingData::object_t object = nullptr;
    
    // This is nullptr if no destruction is needed, or if the object hasn't been constructed yet.
    MultibindingData::destroy_t destroy = nullptr;
  };
  
  // Can be empty, but only if v is present and non-empty.
  std::vector<Elem> elems;
  
  // Returns the std::vector<T*> of instances, or nullptr if none.
  // Caches the result in the `v' member.
  std::shared_ptr<char>(*getSingletonsVector)(InjectorStorage&);
  
  // A (casted) pointer to the std::vector<T*> of singletons, or nullptr if the vector hasn't been constructed yet.
  // Can't be empty.
  std::shared_ptr<char> v;
};


} // namespace impl
} // namespace fruit

#include "binding_data.defn.h"

#endif

