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

#include <fruit/impl/util/type_info.h>
#include <fruit/impl/data_structures/semistatic_graph.h>
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

template <typename Deps>
const BindingDeps* getBindingDeps();

class BindingData {
public:
  using Unsigned = std::uintptr_t;
  
  using object_t = void*;
  using create_t = object_t(*)(InjectorStorage&,
                               SemistaticGraph<TypeId, NormalizedBindingData>::node_iterator);
  
private:
  // `deps' stores the type IDs that this type depends on.
  // If `deps' itself is nullptr, this binding stores an object instead of a create operation.
  const BindingDeps* deps;
  
  // This stores either:
  // 
  // * create, of type create_t if deps!=nullptr
  //   The return type is a pointer to the constructed object (guaranteed to be !=nullptr).
  // 
  // * object, of type object_t if deps==nullptr
  //   The stored object, a casted T*.
  void* p;
  
  // This is false for e.g. bindings, instance bindings that don't need to allocate an object.
  bool needs_allocation;
  
public:
  BindingData() = default;
  
  // Binding data for an object that is not already constructed.
  BindingData(create_t create, const BindingDeps* deps, bool needs_allocation);
    
  // Binding data for an already constructed object.
  BindingData(object_t object);
  
  bool isCreated() const;
  
  // This assumes !isCreated().
  const BindingDeps* getDeps() const;
  
  // This assumes !isCreated().
  create_t getCreate() const;
  
  // This assumes isCreated().
  object_t getObject() const;
  
  bool needsAllocation() const;
  
  bool operator==(const BindingData& other) const;
};

// A CompressedBinding with interface_id==getTypeId<I>() and class_id==getTypeId<C>() means that if:
// * C is not exposed by the component 
// * I is the only node that depends on C
// * There are no multibindings that directly depend on C
// Then binding_data can be used as BindingData for I instead of the BindingData for I, and C can be removed.
struct CompressedBinding {
  TypeId interface_id;
  TypeId class_id;
  BindingData binding_data;
};

class NormalizedBindingData {
private:
  // This stores either:
  // 
  // * create, of type create_t if deps!=nullptr
  //   The return type is a pointer to the constructed object (guaranteed to be !=nullptr).
  // 
  // * object, of type object_t if deps==nullptr
  //   The stored object, a casted T*.
  void* p;
  
public:
  NormalizedBindingData() = default;
  
  explicit NormalizedBindingData(BindingData binding_data);
  
  // Binding data for an object that is not already constructed.
  NormalizedBindingData(BindingData::create_t create);
    
  // Binding data for an already constructed object.
  NormalizedBindingData(BindingData::object_t object);
  
  // This assumes that the graph node is NOT terminal (i.e. that there is no object yet).
  BindingData::create_t getCreate() const;
  
  // This assumes that the graph node is terminal (i.e. that there is an object in this BindingData).
  BindingData::object_t getObject() const;
  
  // This assumes that the graph node is NOT terminal (i.e. that there is no object yet).
  // This changes the graph node to terminal. Registers the destroy operation in InjectorStorage if needed.
  void create(InjectorStorage& storage, 
              typename SemistaticGraph<TypeId, NormalizedBindingData>::node_iterator node_itr);
  
  bool operator==(const NormalizedBindingData& other) const;
};

struct MultibindingData {
  using object_t = void*;
  using destroy_t = void(*)(void*);
  using create_t = object_t(*)(InjectorStorage&);
  using get_multibindings_vector_t = std::shared_ptr<char>(*)(InjectorStorage&);
  
  MultibindingData(create_t create, const BindingDeps* deps, get_multibindings_vector_t get_multibindings_vector, 
                   bool needs_allocation);
  
  MultibindingData(object_t object, get_multibindings_vector_t get_multibindings_vector);
  
  // This is nullptr if the object is already constructed.
  create_t create = nullptr;
  
  // This is nullptr if the object hasn't been constructed yet.
  object_t object = nullptr;
  
  // If object==nullptr (i.e. create!=nullptr), the types that will be injected directly when `create' is called.
  const BindingDeps* deps = nullptr;
  
  // Returns the std::vector<T*> of instances, or nullptr if none.
  // Caches the result in the `v' member of NormalizedMultibindingData.
  get_multibindings_vector_t get_multibindings_vector;

  bool needs_allocation = true;
};

struct NormalizedMultibindingData {
  
  struct Elem {
    explicit Elem(MultibindingData multibinding_data);
    
    // This is nullptr if the object is already constructed.
    MultibindingData::create_t create = nullptr;
    
    // This is nullptr if the object hasn't been constructed yet.
    MultibindingData::object_t object = nullptr;
  };
  
  // Can be empty, but only if v is present and non-empty.
  std::vector<Elem> elems;
  
  // TODO: Check this comment.
  // Returns the std::vector<T*> of instances, or nullptr if none.
  // Caches the result in the `v' member.
  MultibindingData::get_multibindings_vector_t get_multibindings_vector;
  
  // A (casted) pointer to the std::vector<T*> of objects, or nullptr if the vector hasn't been constructed yet.
  // Can't be empty.
  std::shared_ptr<char> v;
};


} // namespace impl
} // namespace fruit

#include <fruit/impl/binding_data.defn.h>

#endif

