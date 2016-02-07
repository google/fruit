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

#include <fruit/component.h>

#include <fruit/impl/injection_errors.h>
#include <fruit/impl/storage/component_storage.h>

#include <memory>

namespace fruit {

template <typename... Params>
template <typename... Bindings>
inline Component<Params...>::Component(PartialComponent<Bindings...> component)
  : storage(std::move(component.storage)) {
  using namespace fruit::impl::meta;
  
  (void)typename CheckIfError<Comp>::type();
  
  // Keep in sync with the Op in PartialComponent::PartialComponent(PartialComponent&&).
  using Op = Eval<Call(ComposeFunctors(Id<ProcessBinding(Bindings)>...,
                                       ProcessDeferredBindings,
                                       ComponentFunctor(ConvertComponent, Comp)),
                       ConstructComponentImpl())>;
  (void)typename CheckIfError<Op>::type();
  
  Op()(storage);
  
#ifndef FRUIT_NO_LOOP_CHECK
  (void)typename CheckIfError<Eval<CheckNoLoopInDeps(typename Op::Result)>>::type();
#endif // !FRUIT_NO_LOOP_CHECK
}

template <typename... Params>
template <typename... OtherParams>
inline Component<Params...>::Component(Component<OtherParams...> component)
  : Component(fruit::createComponent().install(component)) {
}

inline PartialComponent<> createComponent() {
  return {};
}

template <typename... Bindings>
inline PartialComponent<Bindings...>::PartialComponent(fruit::impl::ComponentStorage&& storage)
  : storage(std::move(storage)) {
}

template <typename... Bindings>
template <typename AnnotatedI, typename AnnotatedC>
inline PartialComponent<Bindings..., Bind<AnnotatedI, AnnotatedC>>
PartialComponent<Bindings...>::bind() && {
  return {std::move(storage)};
}

template <typename... Bindings>
template <typename AnnotatedSignature>
inline PartialComponent<Bindings..., RegisterConstructor<AnnotatedSignature>>
PartialComponent<Bindings...>::registerConstructor() && {
  return {std::move(storage)};
}

template <typename... Bindings>
template <typename C>
inline PartialComponent<Bindings..., BindInstance<C>>
PartialComponent<Bindings...>::bindInstance(C& instance) && {
  storage.addBinding(fruit::impl::InjectorStorage::createBindingDataForBindInstance<C, C>(instance));
  return {std::move(storage)};
}

template <typename... Bindings>
template <typename AnnotatedC, typename C>
inline PartialComponent<Bindings..., BindInstance<AnnotatedC>>
PartialComponent<Bindings...>::bindInstance(C& instance) && {
  storage.addBinding(fruit::impl::InjectorStorage::createBindingDataForBindInstance<AnnotatedC, C>(instance));
  return {std::move(storage)};
}

template <typename... Bindings>
template <typename Lambda>
inline PartialComponent<Bindings..., RegisterProvider<Lambda>>
PartialComponent<Bindings...>::registerProvider(Lambda) && {
  return {std::move(storage)};
}

template <typename... Bindings>
template <typename AnnotatedSignature, typename Lambda>
inline PartialComponent<Bindings..., RegisterProvider<AnnotatedSignature, Lambda>>
PartialComponent<Bindings...>::registerProvider(Lambda) && {
  return {std::move(storage)};
}

template <typename... Bindings>
template <typename AnnotatedI, typename AnnotatedC>
inline PartialComponent<Bindings..., AddMultibinding<AnnotatedI, AnnotatedC>>
PartialComponent<Bindings...>::addMultibinding() && {
  return {std::move(storage)};
}

template <typename... Bindings>
template <typename C>
inline PartialComponent<Bindings...>
PartialComponent<Bindings...>::addInstanceMultibinding(C& instance) && {
  auto multibindingData = fruit::impl::InjectorStorage::createMultibindingDataForInstance<C, C>(instance);
  storage.addMultibinding(multibindingData);
  
  return {std::move(storage)};
}

template <typename... Bindings>
template <typename AnnotatedC, typename C>
inline PartialComponent<Bindings...>
PartialComponent<Bindings...>::addInstanceMultibinding(C& instance) && {
  auto multibindingData = fruit::impl::InjectorStorage::createMultibindingDataForInstance<AnnotatedC, C>(instance);
  storage.addMultibinding(multibindingData);
  return {std::move(storage)};
}

template <typename... Bindings>
template <typename C>
inline PartialComponent<Bindings...>
PartialComponent<Bindings...>::addInstanceMultibindings(std::vector<C>& instances) && {
  for (C& instance : instances) {
    auto multibindingData = fruit::impl::InjectorStorage::createMultibindingDataForInstance<C, C>(instance);
    storage.addMultibinding(multibindingData);
  }
  return {std::move(storage)};
}

template <typename... Bindings>
template <typename AnnotatedC, typename C>
inline PartialComponent<Bindings...>
PartialComponent<Bindings...>::addInstanceMultibindings(std::vector<C>& instances) && {
  for (C& instance : instances) {
    auto multibindingData = fruit::impl::InjectorStorage::createMultibindingDataForInstance<AnnotatedC, C>(instance);
    storage.addMultibinding(multibindingData);
  }
  return {std::move(storage)};
}

template <typename... Bindings>
template <typename Lambda>
inline PartialComponent<Bindings..., AddMultibindingProvider<Lambda>>
PartialComponent<Bindings...>::addMultibindingProvider(Lambda) && {
  return {std::move(storage)};
}
  
template <typename... Bindings>
template <typename AnnotatedSignature, typename Lambda>
inline PartialComponent<Bindings..., AddMultibindingProvider<AnnotatedSignature, Lambda>>
PartialComponent<Bindings...>::addMultibindingProvider(Lambda) && {
  return {std::move(storage)};
}
  
template <typename... Bindings>
template <typename DecoratedSignature, typename Lambda>
inline PartialComponent<Bindings..., RegisterFactory<DecoratedSignature, Lambda>>
PartialComponent<Bindings...>::registerFactory(Lambda) && {
  return {std::move(storage)};
}

template <typename... Bindings>
template <typename... OtherCompParams>
inline PartialComponent<Bindings..., InstallComponent<Component<OtherCompParams...>>>
PartialComponent<Bindings...>::install(Component<OtherCompParams...> component) && {
  storage.install(std::move(component.storage));
  return {std::move(storage)};
}

} // namespace fruit


#endif // FRUIT_COMPONENT_DEFN_H
