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

#ifndef FRUIT_PROVIDER_H
#define FRUIT_PROVIDER_H

namespace fruit {
  
namespace impl {

class ComponentStorage;

template <typename C>
struct GetHelper;

} // namespace impl;

template <typename... P>
class Provider {
private:
  using Ps = fruit::impl::List<P...>;
  using Comp = Component<P...>;

  FruitDelegateCheck(fruit::impl::CheckNoRequirementsInProvider<typename Comp::Rs>);
  FruitDelegateChecks(fruit::impl::CheckClassType<P, fruit::impl::GetClassForType<P>>);  
  
  // This is NOT owned by the provider object. It is not deleted on destruction.
  // This is never nullptr.
  fruit::impl::ComponentStorage* storage;
  
  Provider(fruit::impl::ComponentStorage* storage);
  
  friend class fruit::impl::ComponentStorage;
  
  template <typename C>
  friend struct fruit::impl::GetHelper;
  
  template <typename... OtherPs>
  friend class Injector;
  
public:
  template <typename T>
  T get();
  
  /**
   * This is a convenient way to call get(). E.g.:
   * 
   * MyInterface* x(injector);
   * 
   * is equivalent to:
   * 
   * MyInterface* x = injector.get<MyInterface*>();
   */
  template <typename T>
  explicit operator T();
};

} // namespace fruit

#include "impl/provider.templates.h"

#endif // FRUIT_PROVIDER_H
