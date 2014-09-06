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

#ifndef FRUIT_NORMALIZED_COMPONENT_H
#define FRUIT_NORMALIZED_COMPONENT_H

#include "impl/normalized_component_storage.h"

namespace fruit {

template <typename... Params>
class NormalizedComponent {
private:
  fruit::impl::NormalizedComponentStorage storage;
  
  template <typename... OtherParams>
  friend class Injector;
  
public:
  // The Component used as parameter can have (and usually has) unsatisfied requirements, so it is of the form Component<Required<...>, ...>.
  NormalizedComponent(Component<Params...>&& component);
  
  NormalizedComponent(NormalizedComponent&&) = default;
  NormalizedComponent(const NormalizedComponent&) = default;
  
  NormalizedComponent& operator=(NormalizedComponent&&) = default;
  NormalizedComponent& operator=(const NormalizedComponent&) = default;
};

} // namespace fruit

#include "impl/normalized_component.inlines.h"

#endif // FRUIT_NORMALIZED_COMPONENT_H
