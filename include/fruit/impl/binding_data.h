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

#include "type_info.h"
#include <vector>
#include <memory>

namespace fruit {
namespace impl {

class InjectorStorage;

class BindingData {
public:
  using Unsigned = std::uintptr_t;
  
  using object_t = void*;
  using destroy_t = void(*)(InjectorStorage&);
  using create_t = std::pair<object_t, destroy_t>(*)(InjectorStorage&);
  
private:
  // Stores together:
  // * a typeInfo value of type const TypeInfo*
  // * an is_created value of type bool
  // 
  // as (reinterpret_cast<Unsigned>(typeInfo) | is_created)
  Unsigned x1;
  
  // This stores either:
  // 
  // * create, of type create_t if is_created==false
  //   The return type is a pair (constructedObject, destroyOperation), where constructedObject!=null and destroyOperation is
  //   nullptr if no destruction is needed.
  // 
  // * storedSingleton, of type object_t if is_created==true
  //   The stored singleton (if it was already created) or nullptr.
  //   Stores a casted T*.
  Unsigned x2;
  
public:
  BindingData() = default;
  
  // Binding data for a singleton not already constructed.
  BindingData(const TypeInfo* typeInfo, create_t create);
    
  // Binding data for a singleton already constructed.
  BindingData(const TypeInfo* typeInfo, object_t object);
  
  bool isCreated() const;
  
  Unsigned getRawKey() const;
  
  const TypeInfo* getKey() const;
  
  // This assumes !isCreated().
  create_t getCreate() const;
  
  // This assumes isCreated().
  object_t getStoredSingleton() const;
  
  // Assumes !isCreated(). After this call, isCreated()==true. Returns the destroy operation, or nullptr if not needed.
  destroy_t create(InjectorStorage& storage);
  
  bool operator==(const BindingData& other) const;
  
  // Fairly arbitrary lexicographic comparison, needed for std::set.
  bool operator<(const BindingData& other) const;
};

struct BindingDataForMultibinding {
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
  // Caches the result in the `v' member of BindingDataVectorForMultibinding.
  std::shared_ptr<char>(*getSingletonsVector)(InjectorStorage&);
};

struct BindingDataVectorForMultibinding {
  
  struct Elem {
    explicit Elem(BindingDataForMultibinding bindingData) {
      create = bindingData.create;
      object = bindingData.object;
      destroy = bindingData.destroy;
    }
    
    // This is nullptr if the object is already constructed.
    BindingDataForMultibinding::create_t create = nullptr;
    
    // This is nullptr if the object hasn't been constructed yet.
    BindingDataForMultibinding::object_t object = nullptr;
    
    // This is nullptr if no destruction is needed, or if the object hasn't been constructed yet.
    BindingDataForMultibinding::destroy_t destroy = nullptr;
  };
  
  // Can be empty, but only if v is present and non-empty.
  std::vector<Elem> bindingDatas;
  
  // Returns the std::vector<T*> of instances, or nullptr if none.
  // Caches the result in the `v' member.
  std::shared_ptr<char>(*getSingletonsVector)(InjectorStorage&);
  
  // A (casted) pointer to the std::vector<T*> of singletons, or nullptr if the vector hasn't been constructed yet.
  // Can't be empty.
  std::shared_ptr<char> v;
};


} // namespace impl
} // namespace fruit

#endif

