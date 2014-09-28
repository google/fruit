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

#ifndef FRUIT_NORMALIZED_COMPONENT_STORAGE_H
#define FRUIT_NORMALIZED_COMPONENT_STORAGE_H

#include "type_info.h"
#include "semistatic_map.h"
#include "../fruit_forward_decls.h"

#include <set>
#include <memory>
#include <unordered_map>

namespace fruit {
  
namespace impl {

/**
 * Similar to ComponentStorage, but used a normalized representation to minimize the amount
 * of work needed to turn this into an injector. However, adding bindings to a normalized
 * component is slower than adding them to a simple component.
 */
class NormalizedComponentStorage {
public:
  class BindingData {
  public:
    using createArgument_t = void*;
    using object_t = void*;
    using destroy_t = void(*)(object_t);
    using create_t = std::pair<object_t, destroy_t>(*)(InjectorStorage&, createArgument_t);
    
  private:
    // The low-order bit of p1 stores an is_created bool.
    // 
    // All the other bits store either:
    //     
    // * create, of type create_t if is_created==false
    //   A function pointer.
    //   This is NULL if the type wasn't yet bound or if an instance was bound (so storedSingleton!=nullptr).
    //   The return type is a pair (constructedObject, destroyOperation).
    // 
    // * destroy, of type destroy_t if is_created==true
    //   The operation to destroy this singleton, or nullptr if it shouldn't be.
    void* p1;
    
    // This stores either:
    // 
    // * createArgument, of type createArgument_t if is_created==false
    //   This is passed to create() when creating the singleton.
    //   There are no guarantees on the value, it might even be nullptr.
    // 
    // * storedSingleton, of type object_t if is_created==true
    //   The stored singleton (if it was already created) or nullptr.
    //   Stores a casted T*.
    void* p2;
    
  public:
    BindingData() = default;
    
    // Binding data for a singleton not already constructed.
    BindingData(create_t create, createArgument_t createArgument);
      
    // Binding data for a singleton already constructed.
    BindingData(destroy_t destroy, object_t object);
    
    bool isCreated() const;
    
    // These assume !isCreated().
    create_t getCreate() const;
    createArgument_t getCreateArgument() const;
    
    // These assume isCreated().
    destroy_t getDestroy() const;
    object_t getStoredSingleton() const;
    
    // Assumes !isCreated(). After this call, isCreated()==true.
    void create(InjectorStorage& storage);
    
    bool operator==(const BindingData& other) const;
    
    // Fairly arbitrary lexicographic comparison, needed for std::set.
    bool operator<(const BindingData& other) const;
  };
  
  struct BindingDataForMultibinding {
    // Can be empty, but only if s is present and non-empty.
    BindingData bindingData;
    
    // Returns the std::set<T*> of instances, or nullptr if none.
    // Caches the result in the `s' member of BindingDataSetForMultibinding.
    std::shared_ptr<char>(*getSingletonSet)(InjectorStorage&);
  };
  
  struct BindingDataSetForMultibinding {
    // Can be empty, but only if s is present and non-empty.
    std::set<BindingData> bindingDatas;
    
    // Returns the std::set<T*> of instances, or nullptr if none.
    // Caches the result in the `s' member.
    std::shared_ptr<char>(*getSingletonSet)(InjectorStorage&);
    
    // A (casted) pointer to the std::set<T*> of singletons, or nullptr if the set hasn't been constructed yet.
    // Can't be empty.
    std::shared_ptr<char> s;
  };
  
  using BindingVectors = std::pair<std::vector<std::pair<const TypeInfo*, BindingData>>,
                                   std::vector<std::pair<const TypeInfo*, BindingDataForMultibinding>>>;
  
private:
  // Maps the type index of a type T to the corresponding BindingData object.
  SemistaticMap<const TypeInfo*, BindingData> typeRegistry;
  
  // Maps the type index of a type T to a set of the corresponding BindingData objects (for multibindings).
  std::unordered_map<const TypeInfo*, BindingDataSetForMultibinding> typeRegistryForMultibindings;
  
  // The sum of (typeInfo->alignment() + typeInfo->size() - 1) for every binding and multibinding.
  // A new[total_size] allocates enough memory to construct all types registered in this component.
  size_t total_size = 0;
  
  friend class InjectorStorage;
  
public:
  static fruit::impl::NormalizedComponentStorage mergeComponentStorages(fruit::impl::NormalizedComponentStorage&& normalizedStorage,
                                                                        fruit::impl::ComponentStorage&& storage);
  
  NormalizedComponentStorage();
  
  NormalizedComponentStorage(NormalizedComponentStorage&&);
  NormalizedComponentStorage(const NormalizedComponentStorage&) = default;
  
  NormalizedComponentStorage& operator=(NormalizedComponentStorage&&);
  NormalizedComponentStorage& operator=(const NormalizedComponentStorage&);
  
  void swap(NormalizedComponentStorage& other);
  
  NormalizedComponentStorage(BindingVectors&& bindingVectors);
  
  ~NormalizedComponentStorage();
};

extern template SemistaticMap<const TypeInfo*, NormalizedComponentStorage::BindingData>::SemistaticMap(
  typename std::vector<std::pair<const TypeInfo*, NormalizedComponentStorage::BindingData>>::const_iterator first,
  typename std::vector<std::pair<const TypeInfo*, NormalizedComponentStorage::BindingData>>::const_iterator last);

} // namespace impl
} // namespace fruit

#include "normalized_component_storage.inlines.h"

#endif // FRUIT_NORMALIZED_COMPONENT_STORAGE_H
