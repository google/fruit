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

namespace impl {
namespace meta {
// This is a helper class used in the implementation of Component and PartialComponent.
// It's in fruit::impl::meta so that we don't need to qualify everything with fruit::impl::meta.
template <typename... PreviousBindings>
struct OpForComponent {
  template <typename Comp>
  using ConvertTo = Eval<
      Call(ReverseComposeFunctors(Id<ComponentFunctor(ConvertComponent, Comp)>,
		                          ProcessDeferredBindings,
		                          Id<ProcessBinding(PreviousBindings)>...),
           ConstructComponentImpl())>;
  
  template <typename Binding>
  using AddBinding = Eval<
	  Call(ReverseComposeFunctors(Id<ProcessBinding(Binding)>,
		                          Id<ProcessBinding(PreviousBindings)>...),
		   ConstructComponentImpl())>;
};
} // namespace meta
} // namespace impl

template <typename... Params>
template <typename... Bindings>
inline Component<Params...>::Component(PartialComponent<Bindings...> component)
  : storage() {

  (void)typename fruit::impl::meta::CheckIfError<Comp>::type();

  using Op = typename fruit::impl::meta::OpForComponent<Bindings...>::template ConvertTo<Comp>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();

#ifndef FRUIT_NO_LOOP_CHECK
  (void)typename fruit::impl::meta::CheckIfError<fruit::impl::meta::Eval<fruit::impl::meta::CheckNoLoopInDeps(typename Op::Result)>>::type();
#endif // !FRUIT_NO_LOOP_CHECK

  component.storage.addBindings(storage);

  // TODO: re-enable this check somehow.
  // component.component.already_converted_to_component = true;

  Op()(storage);
}

template <typename... Params>
template <typename... OtherParams>
inline Component<Params...>::Component(Component<OtherParams...> component)
  : Component(fruit::createComponent().install(std::move(component))) {
}

template <typename... Bindings>
inline PartialComponent<Bindings...>::~PartialComponent() {
}

template <>
inline PartialComponent<>::~PartialComponent() {

}

inline PartialComponent<> createComponent() {
  return {{}};
}

template <typename... Bindings>
template <typename AnnotatedI, typename AnnotatedC>
inline PartialComponent<fruit::impl::Bind<AnnotatedI, AnnotatedC>, Bindings...>
PartialComponent<Bindings...>::bind() {
  using Op = OpFor<fruit::impl::Bind<AnnotatedI, AnnotatedC>>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();
  
  return {{storage}};
}

template <typename... Bindings>
template <typename AnnotatedSignature>
inline PartialComponent<fruit::impl::RegisterConstructor<AnnotatedSignature>, Bindings...>
PartialComponent<Bindings...>::registerConstructor() {
  using Op = OpFor<fruit::impl::RegisterConstructor<AnnotatedSignature>>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();

  return {{storage}};
}

template <typename... Bindings>
template <typename C>
inline PartialComponent<fruit::impl::BindInstance<C, C>, Bindings...>
PartialComponent<Bindings...>::bindInstance(C& instance) {
  using Op = OpFor<fruit::impl::BindInstance<C, C>>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();
  return {{storage, instance}};
}

template <typename... Bindings>
template <typename AnnotatedC, typename C>
inline PartialComponent<fruit::impl::BindInstance<AnnotatedC, C>, Bindings...>
PartialComponent<Bindings...>::bindInstance(C& instance) {
  using Op = OpFor<fruit::impl::BindInstance<AnnotatedC, C>>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();
  return {{storage, instance}};
}

template <typename... Bindings>
template <typename Lambda>
inline PartialComponent<fruit::impl::RegisterProvider<Lambda>, Bindings...>
PartialComponent<Bindings...>::registerProvider(Lambda) {
  using Op = OpFor<fruit::impl::RegisterProvider<Lambda>>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();
  return {{storage}};
}

template <typename... Bindings>
template <typename AnnotatedSignature, typename Lambda>
inline PartialComponent<fruit::impl::RegisterProvider<AnnotatedSignature, Lambda>, Bindings...>
PartialComponent<Bindings...>::registerProvider(Lambda) {
  using Op = OpFor<fruit::impl::RegisterProvider<AnnotatedSignature, Lambda>>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();
  return {{storage}};
}

template <typename... Bindings>
template <typename AnnotatedI, typename AnnotatedC>
inline PartialComponent<fruit::impl::AddMultibinding<AnnotatedI, AnnotatedC>, Bindings...>
PartialComponent<Bindings...>::addMultibinding() {
  using Op = OpFor<fruit::impl::AddMultibinding<AnnotatedI, AnnotatedC>>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();
  
  return {{storage}};
}

template <typename... Bindings>
template <typename C>
inline PartialComponent<fruit::impl::AddInstanceMultibinding<C>, Bindings...>
PartialComponent<Bindings...>::addInstanceMultibinding(C& instance) {
  return {{storage, instance}};
}

template <typename... Bindings>
template <typename AnnotatedC, typename C>
inline PartialComponent<fruit::impl::AddInstanceMultibinding<AnnotatedC>, Bindings...>
PartialComponent<Bindings...>::addInstanceMultibinding(C& instance) {
  return {{storage, instance}};
}

template <typename... Bindings>
template <typename C>
inline PartialComponent<fruit::impl::AddInstanceVectorMultibindings<C>, Bindings...>
PartialComponent<Bindings...>::addInstanceMultibindings(std::vector<C>& instances) {
  return {{storage, instances}};
}

template <typename... Bindings>
template <typename AnnotatedC, typename C>
inline PartialComponent<fruit::impl::AddInstanceVectorMultibindings<AnnotatedC>, Bindings...>
PartialComponent<Bindings...>::addInstanceMultibindings(std::vector<C>& instances) {
  return {{storage, instances}};
}

template <typename... Bindings>
template <typename Lambda>
inline PartialComponent<fruit::impl::AddMultibindingProvider<Lambda>, Bindings...>
PartialComponent<Bindings...>::addMultibindingProvider(Lambda) {
  using Op = OpFor<fruit::impl::AddMultibindingProvider<Lambda>>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();

  return {{storage}};
}
  
template <typename... Bindings>
template <typename AnnotatedSignature, typename Lambda>
inline PartialComponent<fruit::impl::AddMultibindingProvider<AnnotatedSignature, Lambda>, Bindings...>
PartialComponent<Bindings...>::addMultibindingProvider(Lambda) {
  using Op = OpFor<fruit::impl::AddMultibindingProvider<AnnotatedSignature, Lambda>>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();

  return {{storage}};
}
  
template <typename... Bindings>
template <typename DecoratedSignature, typename Lambda>
inline PartialComponent<fruit::impl::RegisterFactory<DecoratedSignature, Lambda>, Bindings...>
PartialComponent<Bindings...>::registerFactory(Lambda) {
  using Op = OpFor<fruit::impl::RegisterFactory<DecoratedSignature, Lambda>>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();

  return {{storage}};
}

template <typename... Bindings>
inline PartialComponent<Bindings...>::PartialComponent(fruit::impl::PartialComponentStorage<Bindings...> storage)
  : storage(std::move(storage)) {
}

template <typename... Bindings>
template <typename... OtherCompParams>
inline PartialComponent<fruit::impl::InstallComponent<Component<OtherCompParams...>>, Bindings...>
PartialComponent<Bindings...>::install(const Component<OtherCompParams...>& other_component) {
  using Op = OpFor<fruit::impl::InstallComponent<Component<OtherCompParams...>>>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();
  return {{storage, other_component.storage}};
}

} // namespace fruit

#endif // FRUIT_COMPONENT_DEFN_H
