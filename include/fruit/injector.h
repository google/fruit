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

#ifndef FRUIT_INJECTOR_H
#define FRUIT_INJECTOR_H

#include "component.h"
#include "provider.h"
#include "normalized_component.h"

namespace fruit {

template <typename... P>
class Injector {
public:
  // Copying or moving injectors is forbidden.
  Injector(const Injector&) = delete;
  Injector(Injector&&) = delete;
  
  /**
   * Creation of an injector from a component.
   * 
   * Example usage:
   * 
   * Injector<Foo, Bar> injector(getFooBarComponent());
   * Foo* foo(injector); // Equivalent to: Foo* foo = injector.get<Foo*>();
   */
  Injector(Component<P...> component);
  
  /**
   * Creation of an injector from a normalized component.
   * This improves performance when creating many injectors that share the vast majority of the bindings (or even all), by processing the common
   * bindings in advance, and then adding the ones that differ.
   * If most bindings differ the use of NormalizedComponent won't improve performance, use the single-argument constructor instead.
   * 
   * The NormalizedComponent can have requirements, but the PartialComponent can't.
   * 
   * Note that a PartialComponent<...> can NOT be used as second argument, so if the second component is defined inline it must be explicitly casted to
   * the desired Component<...> type.
   * 
   * Example usage:
   * 
   * // At startup
   * NormalizedComponent<Required<Foo, Foo2>, Bar, Bar2> normalizedComponent(getComponent());
   * 
   * ...
   * for (...) {
   *   // This copy can be done before knowing the rest of the bindings (e.g. before a client request is received).
   *   NormalizedComponent<Required<Foo, Foo2>, Bar, Bar2> normalizedComponentCopy = component;
   *   
   *   Request request = ...;
   *   
   *   Injector<Foo, Bar> injector(std::move(component1), getRequestComponent(request));
   *   Foo* foo(injector); // Equivalent to: Foo* foo = injector.get<Foo*>();
   *   ...
   * }
   */
  template <typename... NormalizedComponentParams, typename... ComponentParams>
  Injector(NormalizedComponent<NormalizedComponentParams...> normalizedComponent, Component<ComponentParams...> component);
  
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
  
  /**
   * Gets all multibindings for a type T.
   * 
   * Note that multibindings are independent from bindings, so the binding for T (if any) is not returned.
   * This returns an empty vector if there are no multibindings.
   */
  template <typename T>
  const std::vector<T*>& getMultibindings();
  
  /**
   * Eagerly injects all reachable bindings and multibindings of this injector.
   * Unreachable bindings (i.e. bindings that will never be used anyway) are not processed.
   * Call this if the injector will be shared by multiple threads (directly or through per-thread Providers).
   */
  void eagerlyInjectAll();
  
private:
  using Comp = fruit::impl::ConstructComponentImpl<P...>;

  FruitDelegateCheck(fruit::impl::CheckNoRequirementsInProviderHelper<typename Comp::Rs>);
  FruitDelegateChecks(fruit::impl::CheckClassType<P, fruit::impl::GetClassForType<P>>);  
  
  fruit::impl::InjectorStorage storage;
};

} // namespace fruit


#endif // FRUIT_INJECTOR_H
