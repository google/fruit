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

#ifndef FRUIT_COMPONENT_DEFN_H
#define FRUIT_COMPONENT_DEFN_H

#include "../component.h"

#include "injection_errors.h"
#include "storage/component_storage.h"

#include <memory>

namespace fruit {

template <typename... Params>
template <typename OtherComp>
inline Component<Params...>::Component(PartialComponent<OtherComp> component)
  : PartialComponent<fruit::impl::Apply<fruit::impl::ConstructComponentImpl, Params...>>(std::move(component)) {
}

inline Component<> createComponent() {
  return {};
}

template <typename Comp>
inline PartialComponent<Comp>::PartialComponent(fruit::impl::ComponentStorage&& storage)
  : storage(std::move(storage)) {
}

template <typename Comp>
template <typename SourceComp>
inline PartialComponent<Comp>::PartialComponent(PartialComponent<SourceComp> sourceComponent)
  : storage(std::move(sourceComponent.storage)) {
  fruit::impl::ConvertComponent<Comp, SourceComp>()(storage);
}

template <typename Comp>
template <typename I, typename C>
inline PartialComponent<typename fruit::impl::Bind<Comp, I, C>::Result>
PartialComponent<Comp>::bind() && {
  FruitDelegateCheck(fruit::impl::NotABaseClassOf<I, C>);
  fruit::impl::Bind<Comp, I, C>()(storage);
  return {std::move(storage)};
}

template <typename Comp>
template <typename Signature>
inline PartialComponent<typename fruit::impl::RegisterConstructor<Comp, Signature>::Result>
PartialComponent<Comp>::registerConstructor() && {
  FruitDelegateCheck(fruit::impl::ParameterIsNotASignature<Signature>);
  FruitDelegateCheck(fruit::impl::ConstructorDoesNotExist<Signature>);
  fruit::impl::RegisterConstructor<Comp, Signature>()(storage);
  return {std::move(storage)};
}

template <typename Comp>
template <typename C>
inline PartialComponent<typename fruit::impl::RegisterInstance<Comp, C>::Result>
PartialComponent<Comp>::bindInstance(C& instance) && {
  fruit::impl::RegisterInstance<Comp, C>()(storage, instance);
  return {std::move(storage)};
}

template <typename Comp>
template <typename Function>
inline PartialComponent<typename fruit::impl::RegisterProvider<Comp, Function>::Result>
PartialComponent<Comp>::registerProvider(Function provider) && {
  (void)provider;
  fruit::impl::RegisterProvider<Comp, Function>()(storage);
  return {std::move(storage)};
}

template <typename Comp>
template <typename I, typename C>
inline PartialComponent<typename fruit::impl::AddMultibinding<Comp, I, C>::Result>
PartialComponent<Comp>::addMultibinding() && {
  FruitDelegateCheck(fruit::impl::NotABaseClassOf<I, C>);
  fruit::impl::AddMultibinding<Comp, I, C>()(storage);
  return {std::move(storage)};
}

template <typename Comp>
template <typename C>
inline PartialComponent<typename fruit::impl::AddInstanceMultibinding<Comp, C>::Result>
PartialComponent<Comp>::addInstanceMultibinding(C& instance) && {
  fruit::impl::AddInstanceMultibinding<Comp, C>()(storage, instance);
  return {std::move(storage)};
}

template <typename Comp>
template <typename Function>
inline PartialComponent<
    typename fruit::impl::RegisterMultibindingProvider<Comp, Function>::Result>
PartialComponent<Comp>::addMultibindingProvider(Function provider) && {
  (void)provider;
  fruit::impl::RegisterMultibindingProvider<Comp, Function>()(storage);
  return {std::move(storage)};
}
  
template <typename Comp>
template <typename AnnotatedSignature, typename Lambda>
inline PartialComponent<typename fruit::impl::RegisterFactory<Comp, AnnotatedSignature, Lambda>::Result>
PartialComponent<Comp>::registerFactory(Lambda) && {
  fruit::impl::RegisterFactory<Comp, AnnotatedSignature, Lambda>()(storage);
  return {std::move(storage)};
}

template <typename Comp>
template <typename... OtherCompParams>
inline PartialComponent<
    typename fruit::impl::InstallComponentHelper<Comp, OtherCompParams...>::Result>
PartialComponent<Comp>::install(Component<OtherCompParams...> component) && {
  fruit::impl::InstallComponentHelper<Comp, OtherCompParams...>()(storage, std::move(component.storage));
  return {std::move(storage)};
}

} // namespace fruit


#endif // FRUIT_COMPONENT_DEFN_H
