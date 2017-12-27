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
#include <fruit/impl/fruit_internal_forward_decls.h>
#include <fruit/impl/meta/component.h>
#include <fruit/impl/normalized_component_storage/normalized_component_storage_holder.h>
#include <memory>

namespace fruit {

/**
 * This class allows for fast creation of multiple injectors that share most (or all) the bindings.
 *
 * This is an advanced feature of Fruit that allows to reduce injection time in some cases; if you're just starting to
 * use Fruit you might want to ignore this for now (just construct an Injector from your root Component function).
 *
 * Using a NormalizedComponent only helps if:
 *
 * - You create multiple injectors during the lifetime of a process. E.g. if you only create one injector at startup you
 *   won't benefit from using NormalizedComponent.
 * - Some of those injectors share all (or almost all) their bindings.
 *
 * When both of those requirements apply, you can switch to using NormalizedComponent in the "similar" injectors by
 * first refactoring the injectors' root components to be of the form:
 *
 * fruit::Component<...> getRootComponent(...) {
 *   return fruit::createComponent()
 *       // This contains the bindings common to the group of similar injectors.
 *       .install(getSharedComponent, ...)
 *       // This contains the bindings specific to this injector.
 *       .install(getSpecificComponent, ...);
 * }
 *
 * Then you can change your injector construction from:
 *
 * fruit::Injector<...> injector(getRootComponent, ...);
 *
 * To:
 *
 * fruit::NormalizedComponent<fruit::Required<...>, ...> normalized_component(getSharedComponent, ...);
 * fruit::Injector<...> injector(normalized_component, getSpecificComponent, ...);
 *
 * This splits the work of constructing the Injector in two phases: normalization (where Fruit will call the Component
 * functions to collect all the bindings and check for some classes of runtime errors) and the actual creation of the
 * injector, during which Fruit will also collect/check the additional bindings specific to that injector.
 *
 * Then you can share the same normalized_component object across all those injectors (also in different threads,
 * NormalizedComponent is thread-safe), so that the normalization step only occurs once (i.e., you should only construct
 * NormalizedComponent from getSharedComponent once, otherwise you'd pay the normalization cost multiple times).
 *
 * Creating an Injector from a NormalizedComponent and injecting separate instances is very cheap, on the order of 2 us
 * for an injection graph with 100 classes and 900 edges (for more details see the Benchmarks page of the Fruit wiki:
 * https://github.com/google/fruit/wiki/benchmarks ).
 * This might (depending of course on your performance requirements) allow you to create injectors where it would
 * otherwise be unthinkable, e.g. creating a separate injector for each request in a server.
 *
 * Injectors that share the same NormalizedComponent are still independent; for example, if you call injector.get<Foo>()
 * in two injectors, each injector will construct its own instance of Foo.
 *
 * Example usage in a server:
 *
 * // In the global scope.
 * Component<Request> getRequestComponent(Request* request) {
 *   return fruit::createComponent()
 *       .bindInstance(*request);
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
 *   Injector<Foo, Bar> injector(normalizedComponent, getRequestComponent, &request);
 *   Foo* foo = injector.get<Foo*>();
 *   ...
 * }
 *
 * See also the documentation for the Injector constructor that takes a NormalizedComponent.
 */
template <typename... Params>
class NormalizedComponent {
public:
  /**
   * The Component used as parameter can have (and usually has) unsatisfied requirements, so it's usually of the form
   * Component<Required<...>, ...>.
   *
   * The given component function is called with the provided arguments to construct the root component.
   * The constraints on the argument types (if there are any) are the same as the ones for PartialComponent::install().
   */
  template <typename... FormalArgs, typename... Args>
  NormalizedComponent(Component<Params...> (*)(FormalArgs...), Args&&... args);

  NormalizedComponent(NormalizedComponent&&) = default;
  NormalizedComponent(const NormalizedComponent&) = delete;

  NormalizedComponent& operator=(NormalizedComponent&&) = delete;
  NormalizedComponent& operator=(const NormalizedComponent&) = delete;

private:
  NormalizedComponent(fruit::impl::ComponentStorage&& storage, fruit::impl::MemoryPool memory_pool);

  // This is held via a unique_ptr to avoid including normalized_component_storage.h
  // in fruit.h.
  fruit::impl::NormalizedComponentStorageHolder storage;

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
