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

#ifndef FRUIT_COMPONENT_STORAGE_H
#define FRUIT_COMPONENT_STORAGE_H

#include "metaprogramming.h"
#include "type_info.h"
#include "component.utils.h"
#include "injector_storage.h"

namespace fruit {
  
namespace impl {

template <typename AnnotatedSignature>
struct BindAssistedFactory;

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
    
  using BindingData = InjectorStorage::BindingData;
  using BindingDataForMultibinding = InjectorStorage::BindingDataForMultibinding;
  
  // Maps the type index of a type T to the corresponding BindingData object.
  UnorderedMap<const TypeInfo*, BindingData> typeRegistry;
  
  // Maps the type index of a type T to a set of the corresponding BindingData objects (for multibindings).
  UnorderedMap<const TypeInfo*, BindingDataForMultibinding> typeRegistryForMultibindings;
  
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
                         void* (*create)(InjectorStorage&, void*), 
                         void* createArgument,
                         void (*deleteOperation)(void*));
  
  void createBindingData(const TypeInfo* typeInfo,
                         void* storedSingleton,
                         void (*deleteOperation)(void*));
  
  void createBindingDataForMultibinding(const TypeInfo* typeInfo,
                                        void* (*create)(InjectorStorage&, void*),
                                        void* createArgument,
                                        void (*deleteOperation)(void*),
                                        std::shared_ptr<char>(*createSet)(InjectorStorage&));
  
  void createBindingDataForMultibinding(const TypeInfo* typeInfo,
                                        void* storedSingleton,
                                        void (*deleteOperation)(void*),
                                        std::shared_ptr<char>(*createSet)(InjectorStorage&));
  
  template <typename C>
  void createBindingData(void* (*create)(InjectorStorage&, void*),
                         void* createArgument,
                         void (*deleteOperation)(void*));
  
  template <typename C>
  void createBindingDataForMultibinding(void* (*create)(InjectorStorage&, void*),
                                        void* createArgument,
                                        void (*deleteOperation)(void*));
  
  template <typename C>
  void createBindingData(void* storedSingleton,
                         void (*deleteOperation)(void*));
  
  template <typename C>
  void createBindingDataForMultibinding(void* storedSingleton,
                                        void (*deleteOperation)(void*));
  
  template <typename C>
  static std::shared_ptr<char> createSingletonSet(InjectorStorage& storage);
  
  template <typename... Ts>
  friend class fruit::Injector;
  
  friend class InjectorStorage;
  
public:
  ComponentStorage();
  
  ComponentStorage(const ComponentStorage&) = default;
  ComponentStorage(ComponentStorage&&) = default;
  
  ComponentStorage& operator=(const ComponentStorage& other) = default;
  ComponentStorage& operator=(ComponentStorage&&) = default;
  
  operator InjectorStorage() &&;
  
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
  
  // List<Args...> must be equal to RequiredArgsForAssistedFactory<AnnotatedSignature>.
  template <typename AnnotatedSignature, typename... Args>
  void registerFactory(SignatureType<AnnotatedSignature>(*factory)(Args...));
  
  template <typename I, typename C>
  void addMultibinding();
  
  template <typename C>
  void addInstanceMultibinding(C& instance);
  
  template <typename C, typename... Args>
  void registerMultibindingProvider(C* (*provider)(Args...));
  
  template <typename C, typename... Args>
  void registerMultibindingProvider(C (*provider)(Args...));
  
  void install(const ComponentStorage& other);
};

} // namespace impl
} // namespace fruit

#include "component_storage.inlines.h"
#include "component_storage.templates.h"

#endif // FRUIT_COMPONENT_STORAGE_H
