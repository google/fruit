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

#ifndef FRUIT_COMPONENT_H
#define FRUIT_COMPONENT_H

#include "fruit_forward_decls.h"
#include "impl/component_impl.h"
#include "partial_component.h"

namespace fruit {

// Used to group the requirements of Component.
template <typename... Types>
struct Required {};

// Used to annotate T as a type that uses assisted injection.
template <typename T>
struct Assisted;

/**
 * The parameters must be of the form <P...> or <Required<R...>, P...> where R are the required types and P are the provided ones.
 * If the list of requirements is empty it can be omitted.
 * No type can appear twice. Not even once in R and once in P.
 */
template <typename... Types>
class Component;

template <typename... Types>
class Component : public Component<Required<>, Types...> {
private:
  Component() = default;
  
  friend Component<> createComponent();
  
public:
  
  Component(Component&&) = default;
  Component(const Component&) = default;
  
  /**
   * Converts a component to another, auto-injecting the missing types (if any).
   * This is typically called implicitly when returning a component from a function.
   */
  template <typename Comp>
  Component(Comp&& m) 
    : Component<Required<>, Types...>(std::move(m)) {
  }
};

template <typename... R, typename... P>
class Component<Required<R...>, P...> 
  : public PartialComponent<fruit::impl::List<R...>,
                            fruit::impl::List<P...>,
                            fruit::impl::ConstructDeps<fruit::impl::List<R...>, P...>,
                            fruit::impl::List<>> {
private:
  FruitDelegateCheck(fruit::impl::CheckNoRepeatedTypes<R..., P...>);
  FruitDelegateChecks(fruit::impl::CheckClassType<R, fruit::impl::GetClassForType<R>>);  
  FruitDelegateChecks(fruit::impl::CheckClassType<P, fruit::impl::GetClassForType<P>>);  
  
  using Impl = PartialComponent<fruit::impl::List<R...>,
                                fruit::impl::List<P...>,
                                fruit::impl::ConstructDeps<fruit::impl::List<R...>, P...>,
                                fruit::impl::List<>>;
  
  Component() = default;
  
  template <typename... Types>
  friend class Component;
  
public:
  Component(Component&&) = default;
  Component(const Component&) = default;
  
  template <typename... Params>
  Component(fruit::PartialComponent<Params...>&& component)
    : Impl(std::move(component)) {
  }
};

inline Component<> createComponent() {
  return {};
}


} // namespace fruit

#include "impl/component.templates.h"


#endif // FRUIT_COMPONENT_H
