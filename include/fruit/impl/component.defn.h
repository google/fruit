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
  : PartialComponent<Comp>(std::move(component)) {
  using namespace fruit::impl::meta;
  
  (void)typename CheckIfError<Comp>::type();
  
  // Keep in sync with the Op in PartialComponent::PartialComponent(PartialComponent&&).
  using Op = Eval<Call(ComposeFunctors(ProcessDeferredBindings,
                                       ComponentFunctor(ConvertComponent, Comp)),
                       OtherComp)>;
  (void)typename CheckIfError<Op>::type();
  
#ifndef FRUIT_NO_LOOP_CHECK
  (void)typename CheckIfError<Eval<CheckNoLoopInDeps(typename Op::Result)>>::type();
#endif // !FRUIT_NO_LOOP_CHECK
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
  using namespace fruit::impl::meta;
  
  // Keep in sync with the check in Component::Component(PartialComponent<OtherComp>).
  using Op = Eval<Call(ComposeFunctors(ProcessDeferredBindings,
                                       ComponentFunctor(ConvertComponent, Comp)),
                       SourceComp)>;
  Op()(storage);
}

template <typename Comp>
template <typename AnnotatedI, typename AnnotatedC>
inline PartialComponent<typename PartialComponent<Comp>::template ResultOf<fruit::impl::meta::AddDeferredInterfaceBinding, AnnotatedI, AnnotatedC>>
PartialComponent<Comp>::bind() && {
  using namespace fruit::impl::meta;
  
  using Op = OpFor<AddDeferredInterfaceBinding, AnnotatedI, AnnotatedC>;
  Op()(storage);
  return {std::move(storage)};
}

template <typename Comp>
template <typename AnnotatedSignature>
inline PartialComponent<typename PartialComponent<Comp>::template ResultOf<fruit::impl::meta::DeferredRegisterConstructor, AnnotatedSignature>>
PartialComponent<Comp>::registerConstructor() && {
  using namespace fruit::impl::meta;
  
  using Op = OpFor<DeferredRegisterConstructor, AnnotatedSignature>;
  Op()(storage);
  return {std::move(storage)};
}

template <typename Comp>
template <typename C>
inline PartialComponent<typename PartialComponent<Comp>::template ResultOf<fruit::impl::meta::RegisterInstance, C, C>>
PartialComponent<Comp>::bindInstance(C& instance) && {
  using namespace fruit::impl::meta;
  
  using Op = OpFor<RegisterInstance, C, C>;
  Op()(storage, instance);
  return {std::move(storage)};
}

template <typename Comp>
template <typename AnnotatedC, typename C>
inline PartialComponent<typename PartialComponent<Comp>::template ResultOf<fruit::impl::meta::RegisterInstance, AnnotatedC, C>>
PartialComponent<Comp>::bindInstance(C& instance) && {
  using namespace fruit::impl::meta;
  
  using Op = OpFor<RegisterInstance, AnnotatedC, C>;
  Op()(storage, instance);
  return {std::move(storage)};
}

template <typename Comp>
template <typename Lambda>
inline PartialComponent<typename PartialComponent<Comp>::template ResultOf<fruit::impl::meta::DeferredRegisterProvider, Lambda>>
PartialComponent<Comp>::registerProvider(Lambda lambda) && {
  using namespace fruit::impl::meta;
  
  (void)lambda;
  using Op = OpFor<DeferredRegisterProvider, Lambda>;
  Op()(storage);
  return {std::move(storage)};
}

template <typename Comp>
template <typename AnnotatedSignature, typename Lambda>
inline PartialComponent<typename PartialComponent<Comp>::template ResultOf<fruit::impl::meta::DeferredRegisterProviderWithAnnotations, AnnotatedSignature, Lambda>>
PartialComponent<Comp>::registerProvider(Lambda lambda) && {
  using namespace fruit::impl::meta;
  
  (void)lambda;
  using Op = OpFor<DeferredRegisterProviderWithAnnotations, AnnotatedSignature, Lambda>;
  Op()(storage);
  return {std::move(storage)};
}

template <typename Comp>
template <typename AnnotatedI, typename AnnotatedC>
inline PartialComponent<typename PartialComponent<Comp>::template ResultOf<fruit::impl::meta::AddInterfaceMultibinding, AnnotatedI, AnnotatedC>>
PartialComponent<Comp>::addMultibinding() && {
  using namespace fruit::impl::meta;
  
  using Op = OpFor<AddInterfaceMultibinding, AnnotatedI, AnnotatedC>;
  Op()(storage);
  return {std::move(storage)};
}

template <typename Comp>
template <typename C>
inline PartialComponent<typename PartialComponent<Comp>::template ResultOf<fruit::impl::meta::AddInstanceMultibinding, C, C>>
PartialComponent<Comp>::addInstanceMultibinding(C& instance) && {
  using namespace fruit::impl::meta;
  
  using Op = OpFor<AddInstanceMultibinding, C, C>;
  Op()(storage, instance);
  return {std::move(storage)};
}

template <typename Comp>
template <typename AnnotatedC, typename C>
inline PartialComponent<typename PartialComponent<Comp>::template ResultOf<fruit::impl::meta::AddInstanceMultibinding, AnnotatedC, C>>
PartialComponent<Comp>::addInstanceMultibinding(C& instance) && {
  using namespace fruit::impl::meta;
  
  using Op = OpFor<AddInstanceMultibinding, AnnotatedC, C>;
  Op()(storage, instance);
  return {std::move(storage)};
}

template <typename Comp>
template <typename C>
inline PartialComponent<typename PartialComponent<Comp>::template ResultOf<fruit::impl::meta::AddInstanceMultibindings, C, C>>
PartialComponent<Comp>::addInstanceMultibindings(std::vector<C>& instances) && {
  using namespace fruit::impl::meta;
  
  using Op = OpFor<AddInstanceMultibindings, C, C>;
  Op()(storage, instances);
  return {std::move(storage)};
}

template <typename Comp>
template <typename AnnotatedC, typename C>
inline PartialComponent<typename PartialComponent<Comp>::template ResultOf<fruit::impl::meta::AddInstanceMultibindings, AnnotatedC, C>>
PartialComponent<Comp>::addInstanceMultibindings(std::vector<C>& instances) && {
  using namespace fruit::impl::meta;
  
  using Op = OpFor<AddInstanceMultibindings, AnnotatedC, C>;
  Op()(storage, instances);
  return {std::move(storage)};
}

template <typename Comp>
template <typename Lambda>
inline PartialComponent<typename PartialComponent<Comp>::template ResultOf<fruit::impl::meta::RegisterMultibindingProvider, Lambda>>
PartialComponent<Comp>::addMultibindingProvider(Lambda lambda) && {
  using namespace fruit::impl::meta;
  
  (void)lambda;
  using Op = OpFor<RegisterMultibindingProvider, Lambda>;
  Op()(storage);
  return {std::move(storage)};
}
  
template <typename Comp>
template <typename AnnotatedSignature, typename Lambda>
inline PartialComponent<typename PartialComponent<Comp>::template ResultOf<fruit::impl::meta::RegisterMultibindingProviderWithAnnotations, AnnotatedSignature, Lambda>>
PartialComponent<Comp>::addMultibindingProvider(Lambda lambda) && {
  using namespace fruit::impl::meta;
  
  (void)lambda;
  using Op = OpFor<RegisterMultibindingProviderWithAnnotations, AnnotatedSignature, Lambda>;
  Op()(storage);
  return {std::move(storage)};
}
  
template <typename Comp>
template <typename DecoratedSignature, typename Lambda>
inline PartialComponent<typename PartialComponent<Comp>::template ResultOf<fruit::impl::meta::RegisterFactory, DecoratedSignature, Lambda>>
PartialComponent<Comp>::registerFactory(Lambda) && {
  using namespace fruit::impl::meta;
  
  using Op = OpFor<RegisterFactory, DecoratedSignature, Lambda>;
  Op()(storage);
  return {std::move(storage)};
}

template <typename Comp>
template <typename... OtherCompParams>
inline PartialComponent<typename PartialComponent<Comp>::template ResultOf<fruit::impl::meta::InstallComponentHelper, OtherCompParams...>>
PartialComponent<Comp>::install(Component<OtherCompParams...> component) && {
  using namespace fruit::impl::meta;
  
  using Op = OpFor<InstallComponentHelper, OtherCompParams...>;
  Op()(storage, std::move(component.storage));
  return {std::move(storage)};
}

} // namespace fruit


#endif // FRUIT_COMPONENT_DEFN_H
