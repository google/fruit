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

#ifndef FRUIT_OLD_COMPONENT_STORAGE_H
#define FRUIT_OLD_COMPONENT_STORAGE_H

#include <fruit/impl/util/type_info.h>
#include <fruit/fruit_forward_decls.h>
#include <fruit/impl/bindings/to_port/binding_data.h>
#include <fruit/impl/fruit_internal_forward_decls.h>

#include <forward_list>

namespace fruit {
namespace impl {

/**
 * A component where all types have to be explicitly registered, and all checks are at runtime.
 * Used to implement Component<>, don't use directly.
 * This merely stores the BindingData/CompressedBinding/MultibindingData objects. The real processing will be done in
 * NormalizedComponentStorage and InjectorStorage.
 * 
 * This class handles the creation of types of the forms:
 * - shared_ptr<C>, [const] C*, [const] C&, C (where C is an atomic type)
 * - Annotated<Annotation, T> (with T of the above forms)
 * - Injector<T1, ..., Tk> (with T1, ..., Tk of the above forms).
 */
class OldComponentStorage {
private:  
  // Duplicate elements (elements with the same typeId) are not meaningful and will be removed later.
  std::vector<std::pair<TypeId, BindingData>> bindings;
  
  // All elements in this vector are best-effort. Removing an element from this vector does not affect correctness.
  std::vector<OldCompressedBinding> compressed_bindings;
  
  // Duplicate elements (elements with the same typeId) *are* meaningful, these are multibindings.
  std::vector<std::pair<TypeId, MultibindingData>> multibindings;

  std::vector<OwningGenericLazyComponent> lazy_components;

  template <typename... Ts>
  friend class fruit::Injector;
  
  friend class NormalizedComponentStorage;
  friend class InjectorStorage;
  friend class BindingNormalization;
  friend class LazyComponentExpansionContext;

public:
  OldComponentStorage() = default;
  OldComponentStorage(const OldComponentStorage& other);
  OldComponentStorage(OldComponentStorage&&) = default;

  ~OldComponentStorage();

  OldComponentStorage& operator=(const OldComponentStorage& other);
  OldComponentStorage& operator=(OldComponentStorage&&) = default;

  void addBinding(std::tuple<TypeId, BindingData> t) throw();
  
  // Takes a tuple (getTypeId<I>(), getTypeId<C>(), bindingData)
  void addCompressedBinding(std::tuple<TypeId, TypeId, BindingData> t) throw();
  
  void addMultibinding(std::tuple<TypeId, MultibindingData> t) throw();
  
  void install(OldComponentStorage&& other) throw();

  // This object will take ownership of the pointer.
  void install(LazyComponent* lazy_component) throw();
  void install(LazyComponentWithNoArgs lazy_component) throw();

  std::size_t numBindings() const;
  std::size_t numCompressedBindings() const;
  std::size_t numMultibindings() const;

  void expectBindings(std::size_t n);
  void expectCompressedBindings(std::size_t n);
  void expectMultibindings(std::size_t n);
};

} // namespace impl
} // namespace fruit

#include <fruit/impl/storage/to_port/old_component_storage.defn.h>

#endif // FRUIT_OLD_COMPONENT_STORAGE_H
