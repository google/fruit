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

#include <vector>
#include <unordered_map>
#include "metaprogramming.h"
#include "type_info.h"
#include "module.utils.h"
#include "../fruit_forward_decls.h"

namespace fruit {
  
namespace impl {

template <typename AnnotatedSignature>
struct BindAssistedFactory;

template <typename T>
struct GetHelper;

/**
 * A module where all types have to be explicitly registered, and all checks are at runtime.
 * Used to implement Module<>, don't use directly.
 * 
 * This class handles the creation of types of the forms:
 * - shared_ptr<C>, [const] C*, [const] C&, C (where C is an atomic type)
 * - Injector<T1, ..., Tk> (with T1, ..., Tk of the above forms).
 */
class UnsafeModule {
private:
  
  struct TypeInfo {
    // The stored singleton (if it was already created) or nullptr.
    // Stores a casted T*
    void* storedSingleton;
    
    union {
      // Valid when storedSingleton!=nullptr
      struct {
        // The operation to destroy this singleton, or a no-op if it shouldn't be.
        void (*destroy)(void*);
      };
      
      // Valid when storedSingleton==nullptr
      struct {
        // This is passed to create() when creating the singleton.
        // There are no guarantees on the value, it might even be nullptr.
        void* createArgument;
        
        // A function pointer.
        // This is NULL if and only if the type wasn't yet bound.
        // Returns a pair containing a T* (the created singleton) and the destroy operation.
        std::pair<void*, void(*)(void*)> (*create)(UnsafeModule&, void*);
      };
    };
  };
  
  // The list of types for which a singleton was created, in order of creation.
  // Allows destruction in the correct order.
  // NOTE: instances provided externally via bindInstance() are not in this vector
  // (the order of destruction for them doesn't matter since none of them depend on
  // other singletons).
  std::vector<TypeIndex> createdSingletons;
  
  // Maps the type index of a type T to the corresponding TypeInfo object.
  std::unordered_map<TypeIndex, TypeInfo> typeRegistry;
  
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
  
  void createTypeInfo(TypeIndex typeIndex, 
                      std::pair<void*, void(*)(void*)> (*create)(UnsafeModule&, void*), 
                      void* createArgument);
  
  void createTypeInfo(TypeIndex typeIndex, 
                      void* storedSingleton,
                      void (*deleteOperation)(void*));
  
  template <typename C>
  void createTypeInfo(std::pair<void*, void(*)(void*)> (*create)(UnsafeModule&, void*), 
                      void* createArgument);
  
  template <typename C>
  void createTypeInfo(void* storedSingleton,
                      void (*deleteOperation)(void*));
  
  template <typename C>
  C* getPtr();
  
  void* getPtr(TypeIndex typeIndex);
  
  void clear();
  
  void swap(UnsafeModule& other);
  
  template <typename T>
  friend struct GetHelper;
  
public:
  // When this is called, T and all the types it (recursively) depends on must be bound/registered.
  template <typename T>
  auto get() -> decltype(GetHelper<T>()(*this)) {
    return GetHelper<T>()(*this);
  }
  
  UnsafeModule() = default;
  UnsafeModule(const UnsafeModule& other);
  UnsafeModule(UnsafeModule&& other);
  
  UnsafeModule& operator=(const UnsafeModule& other);
  UnsafeModule& operator=(UnsafeModule&& other);
  
  ~UnsafeModule();
  
  // I, C must not be pointers.
  template <typename I, typename C>
  void bind();
  
  template <typename C>
  void bindInstance(C* instance);
  
  template <typename C, typename... Args>
  void registerProvider(C* (*provider)(Args...));
  
  template <typename C, typename... Args>
  void registerProvider(C (*provider)(Args...));
  
  template <typename AnnotatedSignature>
  void registerFactory(RequiredSignatureForAssistedFactory<AnnotatedSignature>* factory);
  
  // Note: `other' must be a pure module (no singletons created yet)
  // while this doesn't have to be.
  void install(const UnsafeModule& other);
};

} // namespace impl
} // namespace fruit

#include "unsafe_module.inlines.h"
#include "unsafe_module.templates.h"

#endif // FRUIT_UNSAFE_MODULE_H
