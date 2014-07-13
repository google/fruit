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

#ifndef FRUIT_UNSAFE_MODULE_H
#define FRUIT_UNSAFE_MODULE_H

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
  
  struct TypeInfo {
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
    bool operator<(const TypeInfo& other) const;
  };
  
  struct TypeInfoForMultibinding {
    // Can be empty, but only if s is present and non-empty.
    std::set<TypeInfo> typeInfos;
    
    // Returns the std::set<T*> of instances, or nullptr if none.
    // Caches the result in the `s' member.
    std::shared_ptr<char>(*getSingletonSet)(ComponentStorage&);
    
    // A (casted) pointer to the std::set<T*> of singletons, or nullptr if the set hasn't been constructed yet.
    // Can't be empty.
    std::shared_ptr<char> s;
  };
  
  // The list of types for which a singleton was created, in order of creation.
  // Allows destruction in the correct order.
  // NOTE: instances provided externally via bindInstance() are not in this vector
  // (the order of destruction for them doesn't matter since none of them depend on
  // other singletons).
  std::vector<TypeIndex> createdSingletons;
  
  // Maps the type index of a type T to the corresponding TypeInfo object.
  std::unordered_map<TypeIndex, TypeInfo> typeRegistry;
  
  // Maps the type index of a type T to a set of the corresponding TypeInfo objects (for multibindings).
  std::unordered_map<TypeIndex, TypeInfoForMultibinding> typeRegistryForMultibindings;
  
  // A kind of assert(), but always executed. Also prints the message and injected types before aborting.
  // This is inlined so that the compiler knows that this is a no-op if b==false (the usual).
  // It takes a function instead of a std::string so that we don't waste time generating the message in the happy flow.
  template <typename MessageGenerator>
  void check(bool b, MessageGenerator messageGenerator);
  
  // For convenience.
  void check(bool b, const char* message);
  
  void printError(const std::string& message);
  
  template <typename C>
  TypeInfo& getTypeInfo();
  
  template <typename C>
  TypeInfo& getTypeInfoForMultibinding();  
  
  void createTypeInfo(TypeIndex typeIndex, 
                      void* (*create)(ComponentStorage&, void*), 
                      void* createArgument,
                      void (*deleteOperation)(void*));
  
  void createTypeInfo(TypeIndex typeIndex,
                      void* storedSingleton,
                      void (*deleteOperation)(void*));
  
  void createTypeInfoForMultibinding(TypeIndex typeIndex, 
                                     void* (*create)(ComponentStorage&, void*), 
                                     void* createArgument,
                                     void (*deleteOperation)(void*),
                                     std::shared_ptr<char>(*createSet)(ComponentStorage&));
  
  void createTypeInfoForMultibinding(TypeIndex typeIndex,
                                     void* storedSingleton,
                                     void (*deleteOperation)(void*),
                                     std::shared_ptr<char>(*createSet)(ComponentStorage&));
  
  template <typename C>
  void createTypeInfo(void* (*create)(ComponentStorage&, void*), 
                      void* createArgument,
                      void (*deleteOperation)(void*));
  
  template <typename C>
  void createTypeInfoForMultibinding(void* (*create)(ComponentStorage&, void*),
                                     void* createArgument,
                                     void (*deleteOperation)(void*));
  
  template <typename C>
  void createTypeInfo(void* storedSingleton,
                      void (*deleteOperation)(void*));
  
  template <typename C>
  void createTypeInfoForMultibinding(void* storedSingleton,
                                     void (*deleteOperation)(void*));
  
  template <typename C>
  static std::shared_ptr<char> createSingletonSet(ComponentStorage& storage);
  
  template <typename C>
  C* getPtr();
  
  void* getPtr(TypeIndex typeIndex);
  
  void* getPtrForMultibinding(TypeIndex typeIndex);
  
  // Returns a std::set<T*>*, or nullptr if there are no multibindings.
  void* getMultibindings(TypeIndex typeIndex);
  
  void clear();
  
  void swap(ComponentStorage& other);
  
  // Gets the instance from typeInfo, and constructs it if necessary.
  void ensureConstructed(TypeIndex typeIndex, TypeInfo& typeInfo);
  
  // Constructs any necessary instances, but NOT the instance set.
  void ensureConstructedMultibinding(TypeIndex typeIndex, TypeInfoForMultibinding& typeInfoForMultibinding);
  
  template <typename T>
  friend struct GetHelper;
  
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
  void registerProvider(C* (*provider)(Args...), void (*deleter)(void*));
  
  template <typename C, typename... Args>
  void registerProvider(C (*provider)(Args...), void (*deleter)(void*));
  
  template <typename AnnotatedSignature>
  void registerFactory(RequiredSignatureForAssistedFactory<AnnotatedSignature>* factory);
  
  template <typename I, typename C>
  void addMultibinding();
  
  template <typename C>
  void addInstanceMultibinding(C& instance);
  
  template <typename C, typename... Args>
  void registerMultibindingProvider(C* (*provider)(Args...), void (*deleter)(void*));
  
  template <typename C, typename... Args>
  void registerMultibindingProvider(C (*provider)(Args...), void (*deleter)(void*));
  
  // Note: `other' must be a pure component (no singletons created yet)
  // while this doesn't have to be.
  void install(const ComponentStorage& other);
  
  void eagerlyInjectMultibindings();
};

} // namespace impl
} // namespace fruit

#include "component_storage.inlines.h"
#include "component_storage.templates.h"

#endif // FRUIT_UNSAFE_MODULE_H
