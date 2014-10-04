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

#ifndef FRUIT_COMPONENT_TEMPLATES_H
#define FRUIT_COMPONENT_TEMPLATES_H

#include "../component.h"
#include "component_impl.templates.h"

namespace fruit {

template <typename Comp>
inline PartialComponent<Comp>::PartialComponent(fruit::impl::ComponentStorage&& storage)
  : storage(std::move(storage)) {
}

template <typename Comp>
template <typename SourceComp>
inline PartialComponent<Comp>::PartialComponent(const PartialComponent<SourceComp>& sourceComponent)
  : PartialComponent(PartialComponent<SourceComp>(sourceComponent)) {
}

// TODO: Move the implementation of this into a functor.
template <typename Comp>
template <typename SourceComp>
inline PartialComponent<Comp>::PartialComponent(PartialComponent<SourceComp>&& sourceComponent)
  : storage(std::move(sourceComponent.storage)) {
  // We need to register:
  // * All the types provided by the new component
  // * All the types required by the old component
  // except:
  // * The ones already provided by the old component.
  // * The ones required by the new one.
  using ToRegister = fruit::impl::set_difference<fruit::impl::merge_sets<typename Comp::Ps, typename SourceComp::Rs>,
                                    fruit::impl::merge_sets<typename Comp::Rs, typename SourceComp::Ps>>;
  using Helper = fruit::impl::EnsureProvidedTypes<SourceComp, typename Comp::Rs, ToRegister>;
  
  // Not needed, just double-checking.
  // Uses FruitStaticAssert instead of FruitDelegateCheck so that it's checked only in debug mode.
  FruitStaticAssert(true || sizeof(fruit::impl::CheckComponentEntails<typename Helper::Result, Comp>), "");
  
  // Add the required bindings.
  Helper()(storage);
}

} // namespace fruit


#endif // FRUIT_COMPONENT_TEMPLATES_H
