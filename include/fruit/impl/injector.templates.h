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

#ifndef FRUIT_INJECTOR_TEMPLATES_H
#define FRUIT_INJECTOR_TEMPLATES_H

#include "../component.h"

// Redundant, but makes KDevelop happy.
#include "../injector.h"

namespace fruit {

template <typename... P>
Injector<P...>::Injector(const Component<P...>& component)
  : Super(new fruit::impl::ComponentStorage(component.storage)) {
};

template <typename... P>
Injector<P...>::Injector(Component<P...>&& component)
  : Super(new fruit::impl::ComponentStorage(std::move(component.storage))) {
};

template <typename... P>
template <typename ParentProvider, typename ChildComp>
Injector<P...>::Injector(const ParentProvider& parentProvider, const ChildComp& component)
  : Super(new fruit::impl::ComponentStorage(component.storage)) {
  using ThisComp = typename Super::Comp;
  using ParentComp = typename ParentProvider::Comp;
  using Comp1 = decltype(
    fruit::createComponent()
      .install(std::declval<ParentComp>())
      .install(std::declval<ChildComp>()));
  FruitDelegateCheck(fruit::impl::CheckComponentEntails<Comp1, ThisComp>);
  this->storage->setParent(parentProvider.storage);
}

template <typename... P>
template <typename ParentProvider, typename ChildComp>
Injector<P...>::Injector(const ParentProvider& parentProvider, ChildComp&& component)
  : Super(new fruit::impl::ComponentStorage(std::move(component.storage))) {
  using ThisComp = typename Super::Comp;
  using ParentComp = typename ParentProvider::Comp;
  using Comp1 = decltype(
    fruit::createComponent()
      .install(std::declval<ParentComp>())
      .install(std::declval<ChildComp>()));
  FruitDelegateCheck(fruit::impl::CheckComponentEntails<Comp1, ThisComp>);
  this->storage->setParent(parentProvider.storage);
}

template <typename... P>
Injector<P...>::~Injector() {
  delete this->storage;
}

template <typename... P>
template <typename C>
std::set<C*> Injector<P...>::getMultibindings() {
  return this->storage->template getMultibindings<C>();
}

} // namespace fruit


#endif // FRUIT_INJECTOR_TEMPLATES_H
