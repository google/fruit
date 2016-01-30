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

// This include is not required here, but having it here shortens the include trace in error messages.
#include <fruit/impl/injection_errors.h>

#include <fruit/fruit_forward_decls.h>
#include <fruit/impl/storage/normalized_component_storage.h>

namespace fruit {

/**
 * This class allows for fast creation of multiple injectors that share most (or all) the bindings.
 * 
 * Example usage in a server:
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
 *   Foo* foo = injector.get<Foo*>();
 *   ...
 * }
 * 
 * See the 2-argument Injector constructor for more details.
 */
template <typename... Params>
class NormalizedComponent {
public:
  // The Component used as parameter can have (and usually has) unsatisfied requirements, so it's usually of the form
  // Component<Required<...>, ...>.
  NormalizedComponent(Component<Params...>&& component);
  
  NormalizedComponent(NormalizedComponent&&) = default;
  NormalizedComponent(const NormalizedComponent&) = delete;
  
  NormalizedComponent& operator=(NormalizedComponent&&) = delete;
  NormalizedComponent& operator=(const NormalizedComponent&) = delete;
  
private:
  fruit::impl::NormalizedComponentStorage storage;
  
  template <typename... OtherParams>
  friend class Injector;
  
  using Comp = fruit::impl::meta::Eval<fruit::impl::meta::ConstructComponentImpl(fruit::impl::meta::Type<Params>...)>;

  using Check1 = typename fruit::impl::meta::CheckIfError<Comp>::type;
  // Force instantiation of Check1.
  static_assert(true || sizeof(Check1), "");
};

} // namespace fruit

#include <fruit/impl/normalized_component.defn.h>

#endif // FRUIT_NORMALIZED_COMPONENT_H
