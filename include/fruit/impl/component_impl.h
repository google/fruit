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

#ifndef FRUIT_COMPONENT_IMPL_H
#define FRUIT_COMPONENT_IMPL_H

#include "../fruit_forward_decls.h"
#include "component_storage.h"
#include "component.utils.h"
#include "injection_errors.h"
#include "fruit_assert.h"

namespace fruit {

namespace impl {

/**
 * This class contains the methods of Component (which will be used by this library's users) but should not
 * be instantiated directly by client code. I.e. a user of the library should never write `ComponentImpl'.
 * Always start the construction of a component with fruit::createComponent().
 */
template <typename RsParam, typename PsParam, typename DepsParam, typename BindingsParam>
class ComponentImpl {
public:
  // Just for convenience.
  using Rs = RsParam;
  using Ps = PsParam;
  using Deps = DepsParam;
  using Bindings = BindingsParam;
  using This = PartialComponent<Rs, Ps, Deps, Bindings>;
  
  // Invariants:
  // * all types appearing as arguments of Deps are in Rs
  // * all types in Ps are at the head of one (and only one) Dep.
  //   (note that the types in Rs can appear in deps any number of times, 0 is also ok)
  // * Deps is of the form List<Dep...> with each Dep of the form T(Args...) and where List<Args...> is a set (no repetitions).
  // * Bindings is of the form List<ConsBinding<I1, C1>, ..., ConsBinding<In, Cn>> and is a set (no repetitions).
  
  FruitStaticAssert(std::is_same<AddDeps<Deps, List<>>, Deps>::value,
                    "Internal error: ComponentImpl instantiated with non-normalized deps");
  
protected:    
  // Invariant: all types in Ps must be bound in storage.
  ComponentStorage storage;
  
public:
  
  ComponentImpl() = default;
  
  ComponentImpl(ComponentImpl&&) = default;
  ComponentImpl(const ComponentImpl&) = default;
  
  ComponentImpl(ComponentStorage&& storage);
  
  template <typename... Types>
  friend class fruit::Injector;
  
  template <typename... Types>
  friend class fruit::NormalizedComponent;
  
  template <typename... Types>
  friend class fruit::Component;
  
  template <typename... OtherParams>
  friend class fruit::PartialComponent;
  
  template <typename OtherRs, typename OtherPs, typename OtherDeps, typename OtherBindings>
  friend class fruit::impl::ComponentImpl;
  
  template <typename Comp, typename ToRegister>
  friend struct fruit::impl::ComponentConversionHelper;
  
  template <typename Comp, typename TargetRequirements, bool is_already_provided, typename C>
  friend struct fruit::impl::EnsureProvidedTypeHelper;
    
  template <typename Comp>
  friend struct fruit::impl::Identity;
  
  template <typename Comp, typename I, typename C>
  friend struct fruit::impl::Bind;
  
  template <typename Comp, typename I, typename C>
  friend struct fruit::impl::BindNonFactory;
  
  template <typename Comp, typename C>
  friend struct fruit::impl::RegisterInstance;
  
  template <typename Comp, typename Signature>
  friend struct fruit::impl::RegisterProvider;
  
  template <typename Comp, typename I, typename C>
  friend struct fruit::impl::AddMultibinding;
  
  template <typename Comp, typename C>
  friend struct fruit::impl::AddInstanceMultibinding;
  
  template <typename Comp, typename Signature>
  friend struct fruit::impl::RegisterMultibindingProvider;
  
  template <typename Comp, typename AnnotatedSignature>
  friend struct fruit::impl::RegisterFactory;
  
  template <typename Comp, typename Signature>
  friend struct fruit::impl::RegisterConstructor;
  
  template <typename Comp, typename AnnotatedSignature>
  friend struct fruit::impl::RegisterConstructorAsPointerFactory;
  
  template <typename Comp, typename AnnotatedSignature>
  friend struct fruit::impl::RegisterConstructorAsValueFactory;
  
  template <typename Comp, typename OtherM>
  friend struct fruit::impl::InstallComponent;
  
  template <typename Source_Rs, typename Source_Ps, typename Source_Deps, typename Source_Bindings>
  ComponentImpl(ComponentImpl<Source_Rs, Source_Ps, Source_Deps, Source_Bindings>&& sourceComponent);  
  
  template <typename Source_Rs, typename Source_Ps, typename Source_Deps, typename Source_Bindings>
  ComponentImpl(const ComponentImpl<Source_Rs, Source_Ps, Source_Deps, Source_Bindings>& sourceComponent);  
};

} // namespace impl

} // namespace fruit

#endif // FRUIT_COMPONENT_IMPL_H
