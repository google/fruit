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
  
  (void)typename fruit::impl::meta::CheckIfError<Comp>::type();
  
  using Op = Eval<Call(ReverseComposeFunctors(ComponentFunctor(ConvertComponent, Comp),
                                              ProcessDeferredBindings,
                                              Id<ProcessBinding(Bindings)>...),
                       ConstructComponentImpl())>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();
  
  Op()(storage);
  
#ifndef FRUIT_NO_LOOP_CHECK
  (void)typename fruit::impl::meta::CheckIfError<Eval<CheckNoLoopInDeps(typename Op::Result)>>::type();
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
inline PartialComponent<Bind<AnnotatedI, AnnotatedC>, Bindings...>
PartialComponent<Bindings...>::bind() && {
  using Op = fruit::impl::meta::Eval<
      fruit::impl::meta::Call(
          fruit::impl::meta::ReverseComposeFunctors(
              fruit::impl::meta::ProcessBinding(Bind<AnnotatedI, AnnotatedC>),
              fruit::impl::meta::Id<fruit::impl::meta::ProcessBinding(Bindings)>...),
          fruit::impl::meta::ConstructComponentImpl())>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();
  
  return {std::move(storage)};
}

template <typename... Bindings>
template <typename AnnotatedSignature>
inline PartialComponent<RegisterConstructor<AnnotatedSignature>, Bindings...>
PartialComponent<Bindings...>::registerConstructor() && {
  using Op = fruit::impl::meta::Eval<
      fruit::impl::meta::Call(
          fruit::impl::meta::ReverseComposeFunctors(
              fruit::impl::meta::ProcessBinding(RegisterConstructor<AnnotatedSignature>),
              fruit::impl::meta::Id<fruit::impl::meta::ProcessBinding(Bindings)>...),
          fruit::impl::meta::ConstructComponentImpl())>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();

  return {std::move(storage)};
}

template <typename... Bindings>
template <typename C>
inline PartialComponent<BindInstance<C>, Bindings...>
PartialComponent<Bindings...>::bindInstance(C& instance) && {
  using Op = fruit::impl::meta::Eval<
      fruit::impl::meta::Call(
          fruit::impl::meta::ReverseComposeFunctors(
              fruit::impl::meta::ProcessBinding(BindInstance<C>),
              fruit::impl::meta::Id<fruit::impl::meta::ProcessBinding(Bindings)>...),
          fruit::impl::meta::ConstructComponentImpl())>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();
  storage.addBinding(fruit::impl::InjectorStorage::createBindingDataForBindInstance<C, C>(instance));
  return {std::move(storage)};
}

template <typename... Bindings>
template <typename AnnotatedC, typename C>
inline PartialComponent<BindInstance<AnnotatedC>, Bindings...>
PartialComponent<Bindings...>::bindInstance(C& instance) && {
  using Op = fruit::impl::meta::Eval<
      fruit::impl::meta::Call(
          fruit::impl::meta::ReverseComposeFunctors(
              fruit::impl::meta::ProcessBinding(BindInstance<AnnotatedC>),
              fruit::impl::meta::Id<fruit::impl::meta::ProcessBinding(Bindings)>...),
          fruit::impl::meta::ConstructComponentImpl())>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();
  storage.addBinding(fruit::impl::InjectorStorage::createBindingDataForBindInstance<AnnotatedC, C>(instance));
  return {std::move(storage)};
}

template <typename... Bindings>
template <typename Lambda>
inline PartialComponent<RegisterProvider<Lambda>, Bindings...>
PartialComponent<Bindings...>::registerProvider(Lambda) && {
  using Op = fruit::impl::meta::Eval<
      fruit::impl::meta::Call(
          fruit::impl::meta::ReverseComposeFunctors(
              fruit::impl::meta::ProcessBinding(RegisterProvider<Lambda>),
              fruit::impl::meta::Id<fruit::impl::meta::ProcessBinding(Bindings)>...),
          fruit::impl::meta::ConstructComponentImpl())>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();
  return {std::move(storage)};
}

template <typename... Bindings>
template <typename AnnotatedSignature, typename Lambda>
inline PartialComponent<RegisterProvider<AnnotatedSignature, Lambda>, Bindings...>
PartialComponent<Bindings...>::registerProvider(Lambda) && {
  using Op = fruit::impl::meta::Eval<
      fruit::impl::meta::Call(
          fruit::impl::meta::ReverseComposeFunctors(
              fruit::impl::meta::ProcessBinding(RegisterProvider<AnnotatedSignature, Lambda>),
              fruit::impl::meta::Id<fruit::impl::meta::ProcessBinding(Bindings)>...),
          fruit::impl::meta::ConstructComponentImpl())>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();
  return {std::move(storage)};
}

template <typename... Bindings>
template <typename AnnotatedI, typename AnnotatedC>
inline PartialComponent<AddMultibinding<AnnotatedI, AnnotatedC>, Bindings...>
PartialComponent<Bindings...>::addMultibinding() && {
  using Op = fruit::impl::meta::Eval<
      fruit::impl::meta::Call(
          fruit::impl::meta::ReverseComposeFunctors(
              fruit::impl::meta::ProcessBinding(AddMultibinding<AnnotatedI, AnnotatedC>),
              fruit::impl::meta::Id<fruit::impl::meta::ProcessBinding(Bindings)>...),
          fruit::impl::meta::ConstructComponentImpl())>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();
  
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
inline PartialComponent<AddMultibindingProvider<Lambda>, Bindings...>
PartialComponent<Bindings...>::addMultibindingProvider(Lambda) && {
  using Op = fruit::impl::meta::Eval<
      fruit::impl::meta::Call(
          fruit::impl::meta::ReverseComposeFunctors(
              fruit::impl::meta::ProcessBinding(AddMultibindingProvider<Lambda>),
              fruit::impl::meta::Id<fruit::impl::meta::ProcessBinding(Bindings)>...),
          fruit::impl::meta::ConstructComponentImpl())>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();

  return {std::move(storage)};
}
  
template <typename... Bindings>
template <typename AnnotatedSignature, typename Lambda>
inline PartialComponent<AddMultibindingProvider<AnnotatedSignature, Lambda>, Bindings...>
PartialComponent<Bindings...>::addMultibindingProvider(Lambda) && {
  using Op = fruit::impl::meta::Eval<
      fruit::impl::meta::Call(
          fruit::impl::meta::ReverseComposeFunctors(
              fruit::impl::meta::ProcessBinding(AddMultibindingProvider<AnnotatedSignature, Lambda>),
              fruit::impl::meta::Id<fruit::impl::meta::ProcessBinding(Bindings)>...),
          fruit::impl::meta::ConstructComponentImpl())>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();

  return {std::move(storage)};
}
  
template <typename... Bindings>
template <typename DecoratedSignature, typename Lambda>
inline PartialComponent<RegisterFactory<DecoratedSignature, Lambda>, Bindings...>
PartialComponent<Bindings...>::registerFactory(Lambda) && {
  using Op = fruit::impl::meta::Eval<
      fruit::impl::meta::Call(
          fruit::impl::meta::ReverseComposeFunctors(
              fruit::impl::meta::ProcessBinding(RegisterFactory<DecoratedSignature, Lambda>),
              fruit::impl::meta::Id<fruit::impl::meta::ProcessBinding(Bindings)>...),
          fruit::impl::meta::ConstructComponentImpl())>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();

  return {std::move(storage)};
}

template <typename... Bindings>
template <typename... OtherCompParams>
inline PartialComponent<InstallComponent<Component<OtherCompParams...>>, Bindings...>
PartialComponent<Bindings...>::install(Component<OtherCompParams...> component) && {
  using Op = fruit::impl::meta::Eval<
      fruit::impl::meta::Call(
          fruit::impl::meta::ReverseComposeFunctors(
              fruit::impl::meta::ProcessBinding(InstallComponent<Component<OtherCompParams...>>),
              fruit::impl::meta::Id<fruit::impl::meta::ProcessBinding(Bindings)>...),
          fruit::impl::meta::ConstructComponentImpl())>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();

  storage.install(std::move(component.storage));
  return {std::move(storage)};
}

} // namespace fruit


#endif // FRUIT_COMPONENT_DEFN_H
