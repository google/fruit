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

#ifndef FRUIT_INJECTOR_STORAGE_H
#define FRUIT_INJECTOR_STORAGE_H

#include "unordered_map.h"
#include "../fruit_forward_decls.h"

#include <set>
#include <vector>

namespace fruit {
  
namespace impl {

template <typename T>
struct GetHelper;

/**
 * A component where all types have to be explicitly registered, and all checks are at runtime.
 * Used to implement Component<>, don't use directly.
 * 
 * This class handles the creation of types of the forms:
 * - shared_ptr<C>, [const] C*, [const] C&, C (where C is an atomic type)
 * - Injector<T1, ..., Tk> (with T1, ..., Tk of the above forms).
 */
class InjectorStorage {
public:
  struct BindingData {
    // The stored singleton (if it was already created) or nullptr.
    // Stores a casted T*
    void* storedSingleton;
    
    // A function pointer.
    // This is NULL if the type wasn't yet bound or if an instance was bound (so storedSingleton!=nullptr).
    void* (*create)(InjectorStorage&, void*);
    
    // This is passed to create() when creating the singleton.
    // There are no guarantees on the value, it might even be nullptr.
    void* createArgument;
    
    // The operation to destroy this singleton, or a no-op if it shouldn't be.
    void (*destroy)(void*);
    
    // Fairly arbitrary lexicographic comparison, needed for std::set.
    bool operator<(const BindingData& other) const;
  };
  
  struct BindingDataForMultibinding {
    // Can be empty, but only if s is present and non-empty.
    std::set<BindingData> bindingDatas;
    
    // Returns the std::set<T*> of instances, or nullptr if none.
    // Caches the result in the `s' member.
    std::shared_ptr<char>(*getSingletonSet)(InjectorStorage&);
    
    // A (casted) pointer to the std::set<T*> of singletons, or nullptr if the set hasn't been constructed yet.
    // Can't be empty.
    std::shared_ptr<char> s;
  };
  
private:
  // A chunk of memory used to avoid multiple allocations, since we know all sizes when the injector is created, and the number of used bytes.
  char* singletonStorageBegin = nullptr;
  size_t singletonStorageNumUsedBytes = 0;
  
  // The list of types for which a singleton was created, in order of creation.
  // Allows destruction in the correct order.
  // NOTE: instances provided externally via bindInstance() are not in this vector
  // (the order of destruction for them doesn't matter since none of them depend on
  // other singletons).
  std::vector<const TypeInfo*> createdSingletons;
  
  // Maps the type index of a type T to the corresponding BindingData object.
  UnorderedMap<const TypeInfo*, BindingData> typeRegistry;
  
  // Maps the type index of a type T to a set of the corresponding BindingData objects (for multibindings).
  UnorderedMap<const TypeInfo*, BindingDataForMultibinding> typeRegistryForMultibindings;
  
  template <typename C>
  BindingData& getBindingData();
  
  template <typename C>
  BindingDataForMultibinding& getBindingDataForMultibinding();  
  
  template <typename C>
  C* getPtr();
  
  void* getPtr(const TypeInfo* typeInfo);
  
  void* getPtrForMultibinding(const TypeInfo* typeInfo);
  
  // Returns a std::set<T*>*, or nullptr if there are no multibindings.
  void* getMultibindings(const TypeInfo* typeInfo);
  
  void clear();
  
  // Gets the instance from bindingData, and constructs it if necessary.
  void ensureConstructed(const TypeInfo* typeInfo, BindingData& bindingData);
  
  // Constructs any necessary instances, but NOT the instance set.
  void ensureConstructedMultibinding(const TypeInfo* typeInfo, BindingDataForMultibinding& bindingDataForMultibinding);
  
  template <typename T>
  friend struct GetHelper;
  
  friend class ComponentStorage;
  
  // A kind of assert(), but always executed. Also prints the message and injected types before aborting.
  // This is inlined so that the compiler knows that this is a no-op if b==false (the usual).
  // It takes a function instead of a std::string so that we don't waste time generating the message in the happy flow.
  template <typename MessageGenerator>
  void check(bool b, MessageGenerator messageGenerator);
  
  // For convenience.
  void check(bool b, const char* message);
  
  void printError(const std::string& message);
  
public:
  InjectorStorage(UnorderedMap<const TypeInfo*, BindingData>&& typeRegistry,
                  UnorderedMap<const TypeInfo*, BindingDataForMultibinding>&& typeRegistryForMultibindings);
  
  InjectorStorage(InjectorStorage&&) = default;
  InjectorStorage& operator=(InjectorStorage&&) = default;
  
  InjectorStorage(const InjectorStorage& other) = delete;
  InjectorStorage& operator=(const InjectorStorage& other) = delete;
  
  ~InjectorStorage();
  
  // When this is called, T and all the types it (recursively) depends on must be bound/registered.
  template <typename T>
  auto get() -> decltype(GetHelper<T>()(*this)) {
    return GetHelper<T>()(*this);
  }
  
  template <typename C, typename... Args>
  C* constructSingleton(Args&&... args);
  
  template <typename C>
  std::set<C*> getMultibindings();
  
  void eagerlyInjectMultibindings();
};

} // namespace impl
} // namespace fruit

#include "injector_storage.inlines.h"
#include "injector_storage.templates.h"


#endif // FRUIT_INJECTOR_STORAGE_H
