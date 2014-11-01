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

#include "component.h"

namespace fruit {

template <typename... P>
class Provider {
public:
  /**
   * Returns an instance of the specified type. For any class C in the Provider's template parameters, the following variations
   * are allowed:
   * 
   * get<C>()
   * get<C*>()
   * get<C&>()
   * get<const C*>()
   * get<const C&>()
   * get<shared_ptr<C>>()
   * 
   * The shared_ptr version comes with a slight performance hit, avoid it if possible.
   * Calling get<> repeatedly for the same class with the same injector will return the same instance.
   */
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
  
private:
  using Comp = fruit::impl::Apply<fruit::impl::ConstructComponentImpl, P...>;

  FruitDelegateCheck(fruit::impl::CheckNoRequirementsInProviderHelper<typename Comp::Rs>);
  FruitDelegateChecks(fruit::impl::CheckClassType<P, fruit::impl::Apply<fruit::impl::GetClassForType, P>>);  
  
  // This is NOT owned by the provider object. It is not deleted on destruction.
  // This is never nullptr.
  fruit::impl::InjectorStorage* storage;
  
  Provider(fruit::impl::InjectorStorage* storage);
  
  friend class fruit::impl::InjectorStorage;
  
  template <typename C>
  friend struct fruit::impl::GetHelper;
  
  template <typename... OtherPs>
  friend class Injector;
};

} // namespace fruit

#include "impl/provider.defn.h"

#endif // FRUIT_PROVIDER_H
