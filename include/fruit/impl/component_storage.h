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

#ifndef FRUIT_UNSAFE_COMPONENT_H
#define FRUIT_UNSAFE_COMPONENT_H

#include "metaprogramming.h"
#include "type_info.h"
#include "component.utils.h"
#include "../fruit_forward_decls.h"

#include <vector>
#include <unordered_map>
#include <set>

namespace fruit {
  
namespace impl {

template <typename AnnotatedSignature>
struct BindAssistedFactory;

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
class ComponentStorage {
private:
  
  struct BindingData {
    // The stored singleton (if it was already created) or nullptr.
    // Stores a casted T*
    void* storedSingleton;
    
    // A function pointer.
    // This is NULL if the type wasn't yet bound or if an instance was bound (so storedSingleton!=nullptr).
    void* (*create)(ComponentStorage&, void*);
    
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
    std::shared_ptr<char>(*getSingletonSet)(ComponentStorage&);
    
    // A (casted) pointer to the std::set<T*> of singletons, or nullptr if the set hasn't been constructed yet.
    // Can't be empty.
    std::shared_ptr<char> s;
  };
  
  // A chunk of memory used to avoid multiple allocations, since we know all sizes when the injector is created, and the number of used bytes.
  // These are (respectively) nullptr and 0 for pure components.
  char* singletonStorageBegin = nullptr;
  size_t singletonStorageNumUsedBytes = 0;
  
  // The list of types for which a singleton was created, in order of creation.
  // Allows destruction in the correct order.
  // NOTE: instances provided externally via bindInstance() are not in this vector
  // (the order of destruction for them doesn't matter since none of them depend on
  // other singletons).
  std::vector<const TypeInfo*> createdSingletons;
  
  // Maps the type index of a type T to the corresponding BindingData object.
  std::unordered_map<const TypeInfo*, BindingData> typeRegistry;
  
  // Maps the type index of a type T to a set of the corresponding BindingData objects (for multibindings).
  std::unordered_map<const TypeInfo*, BindingDataForMultibinding> typeRegistryForMultibindings;
  
  // A kind of assert(), but always executed. Also prints the message and injected types before aborting.
  // This is inlined so that the compiler knows that this is a no-op if b==false (the usual).
  // It takes a function instead of a std::string so that we don't waste time generating the message in the happy flow.
  template <typename MessageGenerator>
  void check(bool b, MessageGenerator messageGenerator);
  
  // For convenience.
  void check(bool b, const char* message);
  
  void printError(const std::string& message);
  
  template <typename C>
  BindingData& getBindingData();
  
  template <typename C>
  BindingData& getBindingDataForMultibinding();  
  
  void createBindingData(const TypeInfo* typeInfo,
                         void* (*create)(ComponentStorage&, void*), 
                         void* createArgument,
                         void (*deleteOperation)(void*));
  
  void createBindingData(const TypeInfo* typeInfo,
                         void* storedSingleton,
                         void (*deleteOperation)(void*));
  
  void createBindingDataForMultibinding(const TypeInfo* typeInfo,
                                        void* (*create)(ComponentStorage&, void*),
                                        void* createArgument,
                                        void (*deleteOperation)(void*),
                                        std::shared_ptr<char>(*createSet)(ComponentStorage&));
  
  void createBindingDataForMultibinding(const TypeInfo* typeInfo,
                                        void* storedSingleton,
                                        void (*deleteOperation)(void*),
                                        std::shared_ptr<char>(*createSet)(ComponentStorage&));
  
  template <typename C>
  void createBindingData(void* (*create)(ComponentStorage&, void*),
                         void* createArgument,
                         void (*deleteOperation)(void*));
  
  template <typename C>
  void createBindingDataForMultibinding(void* (*create)(ComponentStorage&, void*),
                                        void* createArgument,
                                        void (*deleteOperation)(void*));
  
  template <typename C>
  void createBindingData(void* storedSingleton,
                         void (*deleteOperation)(void*));
  
  template <typename C>
  void createBindingDataForMultibinding(void* storedSingleton,
                                        void (*deleteOperation)(void*));
  
  template <typename C>
  static std::shared_ptr<char> createSingletonSet(ComponentStorage& storage);
  
  template <typename C>
  C* getPtr();
  
  void* getPtr(const TypeInfo* typeInfo);
  
  void* getPtrForMultibinding(const TypeInfo* typeInfo);
  
  // Returns a std::set<T*>*, or nullptr if there are no multibindings.
  void* getMultibindings(const TypeInfo* typeInfo);
  
  void clear();
  
  void swap(ComponentStorage& other);
  
  // Gets the instance from bindingData, and constructs it if necessary.
  void ensureConstructed(const TypeInfo* typeInfo, BindingData& bindingData);
  
  // Constructs any necessary instances, but NOT the instance set.
  void ensureConstructedMultibinding(const TypeInfo* typeInfo, BindingDataForMultibinding& bindingDataForMultibinding);
  
  // Call this when the component becomes an injector.
  // Bindings can only be added before calling this method; injections can only be done after calling this.
  void becomeInjector();
  
  template <typename C, typename... Args>
  C* constructSingleton(Args... args);
  
  template <typename T>
  friend struct GetHelper;
  
  template <typename... Ts>
  friend class fruit::Injector;
  
public:
  // When this is called, T and all the types it (recursively) depends on must be bound/registered.
  template <typename T>
  auto get() -> decltype(GetHelper<T>()(*this)) {
    return GetHelper<T>()(*this);
  }
  
  template <typename C>
  std::set<C*> getMultibindings();
  
  ComponentStorage() = default;
  ComponentStorage(const ComponentStorage& other);
  ComponentStorage(ComponentStorage&& other);
  
  ComponentStorage& operator=(const ComponentStorage& other);
  ComponentStorage& operator=(ComponentStorage&& other);
  
  ~ComponentStorage();
  
  // I, C must not be pointers.
  template <typename I, typename C>
  void bind();
  
  template <typename C>
  void bindInstance(C& instance);
  
  template <typename C, typename... Args>
  void registerProvider(C* (*provider)(Args...));
  
  template <typename C, typename... Args>
  void registerProvider(C (*provider)(Args...));
  
  template <typename C, typename... Args>
  void registerConstructor();
  
  template <typename AnnotatedSignature>
  void registerFactory(RequiredSignatureForAssistedFactory<AnnotatedSignature>* factory);
  
  template <typename I, typename C>
  void addMultibinding();
  
  template <typename C>
  void addInstanceMultibinding(C& instance);
  
  template <typename C, typename... Args>
  void registerMultibindingProvider(C* (*provider)(Args...));
  
  template <typename C, typename... Args>
  void registerMultibindingProvider(C (*provider)(Args...));
  
  // Note: `other' must be a pure component (no singletons created yet)
  // while this doesn't have to be.
  void install(const ComponentStorage& other);
  
  void eagerlyInjectMultibindings();
};

} // namespace impl
} // namespace fruit

#include "component_storage.inlines.h"
#include "component_storage.templates.h"

#endif // FRUIT_UNSAFE_COMPONENT_H
