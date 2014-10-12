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

/**
 * An injector is a class constructed from a component that performs the needed injections and manages the lifetime of the created
 * objects.
 * 
 * 
 * Example usage:
 * 
 * Injector<Foo, Bar> injector(getFooBarComponent());
 * Foo* foo = injector.get<Foo*>();
 * Bar* bar(injector); // Equivalent to: Bar* bar = injector.get<Bar*>();
 */
template <typename... P>
class Injector {
public:
  // Copying injectors is forbidden.
  Injector(const Injector&) = delete;
  
  // Moving injectors is allowed.
  Injector(Injector&&) = default;
  
  /**
   * Creation of an injector from a component.
   * 
   * Example usage:
   * 
   * Injector<Foo, Bar> injector(getFooBarComponent());
   * Foo* foo = injector.get<Foo*>();
   * Bar* bar(injector); // Equivalent to: Bar* bar = injector.get<Bar*>();
   */
  Injector(Component<P...> component);
  
  /**
   * Creation of an injector from a normalized component and a component.
   * 
   * This constructor can be used to improve performance when creating many injectors that share the vast majority of the bindings
   * (or even all), by processing the common bindings in advance, and then adding the ones that differ.
   * If most bindings differ, the use of NormalizedComponent and this constructor won't improve performance; in that case, use the
   * single-argument constructor instead.
   * 
   * The NormalizedComponent can have requirements, but the Component can't.
   * 
   * Note that a PartialComponent<...> can NOT be used as second argument, so if the second component is defined inline it must be
   * explicitly casted to the desired Component<...> type.
   * 
   * Example usage:
   * 
   * // In the global scope.
   * Component<Request> getRequestComponent(Request& request) {
   *   return fruit::createComponent()
   *       .bindInstance(request);
   * }
   * 
   * // At startup (e.g. inside main()).
   * NormalizedComponent<Required<Request>, Bar, Bar2> normalizedComponent = ...;
   * 
   * ...
   * for (...) {
   *   // For each request.
   *   Request request = ...;
   *   
   *   Injector<Foo, Bar> injector(normalizedComponent, getRequestComponent(request));
   *   Foo* foo(injector); // Equivalent to: Foo* foo = injector.get<Foo*>();
   *   ...
   * }
   */
  template <typename... NormalizedComponentParams, typename... ComponentParams>
  Injector(NormalizedComponent<NormalizedComponentParams...> normalizedComponent, Component<ComponentParams...> component);
  
  /**
   * Returns an instance of the specified type. For any class C in the Injector's template parameters, the following variations
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
   * Multibindings are independent from bindings; so if there is a (normal) binding for T that is not returned.
   * This returns an empty vector if there are no multibindings.
   */
  template <typename T>
  const std::vector<T*>& getMultibindings();
  
  /**
   * Eagerly injects all reachable bindings and multibindings of this injector.
   * This only creates instances of the types that are either:
   * - exposed by this Injector (i.e. in the Injector's type parameters)
   * - all multibindings
   * - needed to inject one of the above (directly or indirectly)
   * 
   * Unreachable bindings (i.e. bindings that are not exposed by this Injector, and that are not used by any reachable binding)
   * are not processed.
   * 
   * Call this to ensure thread safety if the injector will be shared by multiple threads.
   * After calling this method, get() and getMultibindings() can be called concurrently on the same injector, with no locking.
   * Note that the guarantee only applies after this method returns; specifically, this method can NOT be called concurrently
   * unless it has been called before on the same injector and returned.
   */
  void eagerlyInjectAll();
  
private:
  using Comp = fruit::impl::ConstructComponentImpl<P...>;

  FruitDelegateCheck(fruit::impl::CheckNoRequirementsInProviderHelper<typename Comp::Rs>);
  FruitDelegateChecks(fruit::impl::CheckClassType<P, fruit::impl::GetClassForType<P>>);  
  
  std::unique_ptr<fruit::impl::InjectorStorage> storage;
};

} // namespace fruit


#endif // FRUIT_INJECTOR_H
