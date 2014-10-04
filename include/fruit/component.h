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

/**
 * The parameters must be of the form <P...> or <Required<R...>, P...> where R are the required types and P are the provided ones.
 * If the list of requirements is empty it can be omitted.
 * No type can appear twice. Not even once in R and once in P.
 */
template <typename... Params>
class Component : public PartialComponent<fruit::impl::ConstructComponentImpl<Params...>> {
private:
  using Comp = fruit::impl::ConstructComponentImpl<Params...>;
  using Base = PartialComponent<Comp>;
  
  // Use createComponent() instead.
  Component() = default;
  
  friend Component<> createComponent();
  
public:
  Component(Component&&) = default;
  Component(const Component&) = default;
  
  /**
   * Converts a PartialComponent (or Component) to an arbitrary Component, auto-injecting the missing types (if any).
   * This is usually called implicitly when returning a component from a function.
   */
  template <typename OtherComp>
  Component(fruit::PartialComponent<OtherComp>&& component)
    : Base(std::move(component)) {
  }
  
  /**
   * Converts a PartialComponent (or Component) to an arbitrary Component, auto-injecting the missing types (if any).
   * This is usually called implicitly when returning a component from a function.
   */
  template <typename OtherComp>
  Component(const fruit::PartialComponent<OtherComp>& component)
    : Base(std::move(component)) {
  }
};

inline Component<> createComponent() {
  return {};
}


} // namespace fruit

#include "impl/component.templates.h"


#endif // FRUIT_COMPONENT_H
