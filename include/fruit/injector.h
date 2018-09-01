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

// This include is not required here, but having it here shortens the include trace in error messages.
#include <fruit/impl/injection_errors.h>

#include <fruit/component.h>
#include <fruit/normalized_component.h>
#include <fruit/provider.h>
#include <fruit/impl/meta_operation_wrappers.h>

namespace fruit {

/**
 * An injector is a class constructed from a component that performs the needed injections and manages the lifetime of
 * the created objects.
 * An injector does *not* need to specify all types bound in the component; you can only specify the "root" type(s) and
 * the injector will also create and store the instances of classes that are needed (directly or indirectly) to inject
 * the root types.
 *
 * Example usage:
 *
 * Component<Foo, Bar> getFooBarComponent() {
 *   ...
 * }
 *
 * Injector<Foo, Bar> injector(getFooBarComponent);
 * Foo* foo = injector.get<Foo*>();
 * Bar* bar(injector); // Equivalent to: Bar* bar = injector.get<Bar*>();
 */
template <typename... P>
class Injector {
public:
  // Moving injectors is allowed.
  Injector(Injector&&) = default;

  // Copying injectors is forbidden.
  Injector(const Injector&) = delete;

  /**
   * This creates an injector from a component function (that can optionally have parameters).
   *
   * Args and FormalArgs (if any) must be the same types; or to be precise, each type in Args must be convertible into
   * the corresponding type in FormalArgs.
   *
   * Example usage:
   *
   * Component<Foo, Bar> getFooBarComponent() {
   *   ...
   * }
   *
   * Injector<Foo, Bar> injector(getFooBarComponent);
   * Foo* foo = injector.get<Foo*>();
   * Bar* bar(injector); // Equivalent to: Bar* bar = injector.get<Bar*>();
   *
   * Example usage with arguments:
   *
   * Component<Foo, Bar> getFooBarComponent(int n, double d) {
   *   ...
   * }
   *
   * Injector<Foo, Bar> injector(getFooBarComponent, 10, 3.14);
   * Foo* foo = injector.get<Foo*>();
   */
  template <typename... FormalArgs, typename... Args>
  Injector(Component<P...> (*)(FormalArgs...), Args&&... args);

  /**
   * This creates an injector from a normalized component and a component function.
   * See the documentation of NormalizedComponent for more details.
   *
   * Args and FormalArgs (if any) must be the same types; or to be precise, each type in Args must be convertible into
   * the corresponding type in FormalArgs.
   *
   * The NormalizedComponent can have requirements, but the Component can't.
   * The NormalizedComponent must remain valid during the lifetime of any Injector object constructed with it.
   *
   * Example usage:
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
   */
  template <typename... NormalizedComponentParams, typename... ComponentParams, typename... FormalArgs,
            typename... Args>
  Injector(const NormalizedComponent<NormalizedComponentParams...>& normalized_component,
           Component<ComponentParams...> (*)(FormalArgs...), Args&&... args);

  /**
   * Deleted constructor, to ensure that constructing an Injector from a temporary NormalizedComponent doesn't compile.
   */
  template <typename... NormalizedComponentParams, typename... ComponentParams, typename... FormalArgs,
            typename... Args>
  Injector(NormalizedComponent<NormalizedComponentParams...>&& normalized_component,
           Component<ComponentParams...> (*)(FormalArgs...), Args&&... args) = delete;

  /**
   * Returns an instance of the specified type. For any class C in the Injector's template parameters, the following
   * variations are allowed:
   *
   * get<C>()
   * get<C*>()
   * get<C&>()
   * get<const C*>()
   * get<const C&>()
   * get<shared_ptr<C>>()
   * get<Provider<C>>()
   * get<Provider<const C>>()
   * get<Annotated<Annotation, C>>()                   (for any type `Annotation')
   * get<Annotated<Annotation, C*>>()                  (for any type `Annotation')
   * get<Annotated<Annotation, C&>>()                  (for any type `Annotation')
   * get<Annotated<Annotation, const C*>>()            (for any type `Annotation')
   * get<Annotated<Annotation, const C&>>()            (for any type `Annotation')
   * get<Annotated<Annotation, shared_ptr<C>>>()       (for any type `Annotation')
   * get<Annotated<Annotation, Provider<C>>>()         (for any type `Annotation')
   * get<Annotated<Annotation, Provider<const C>>>()   (for any type `Annotation')
   *
   * For any "const C" in the Injector's template parameters, only a subset of those are allowed, specifically:
   *
   * get<C>()
   * get<const C*>()
   * get<const C&>()
   * get<Provider<const C>>()
   * get<Annotated<Annotation, C>>()                   (for any type `Annotation')
   * get<Annotated<Annotation, const C*>>()            (for any type `Annotation')
   * get<Annotated<Annotation, const C&>>()            (for any type `Annotation')
   * get<Annotated<Annotation, Provider<const C>>>()   (for any type `Annotation')
   *
   * With a non-annotated parameter T, this returns a T.
   * With an annotated parameter AnnotatedT=Annotated<Annotation, T>, this returns a T.
   * E.g. if you want to inject a pointer for an annotated type, you can use this as follows:
   *
   * T* instance = injector.get<Annotated<Annotation, T*>>();
   *
   * The shared_ptr versions come with a slight performance hit, prefer injecting pointers or references if possible.
   * Calling get<> repeatedly for the same class with the same injector will return the same instance.
   */
  template <typename T>
  fruit::impl::RemoveAnnotations<T> get();

  /**
   * This is a convenient way to call get(). E.g.:
   *
   * MyInterface* x(injector);
   *
   * is equivalent to:
   *
   * MyInterface* x = injector.get<MyInterface*>();
   *
   * Note that this can't be used to inject an annotated type, i.e. this does NOT work:
   *
   * fruit::Annotated<SomeAnnotation, SomeClass> foo(injector);
   *
   * Because foo would be of type fruit::Annotated, not of type SomeClass. In that case you must use get() instead,
   * e.g.:
   *
   * SomeClass* foo = injector.get<fruit::Annotated<SomeAnnotation, SomeClass*>>();;
   */
  template <typename T>
  explicit operator T();

  /**
   * Gets all multibindings for a type T.
   *
   * Multibindings are independent from bindings; so if there is a (normal) binding for T, that is not returned.
   * This returns an empty vector if there are no multibindings.
   *
   * With a non-annotated parameter T, this returns a const std::vector<T*>&.
   * With an annotated parameter AnnotatedT=Annotated<Annotation, T>, this returns a const std::vector<T*>&.
   */
  template <typename T>
  const std::vector<fruit::impl::RemoveAnnotations<T>*>& getMultibindings();

  /**
   * This method is deprecated since Fruit injectors can now be accessed concurrently by multiple threads. This will be
   * removed in a future Fruit release.
   *
   * Eagerly injects all reachable bindings and multibindings of this injector.
   * This only creates instances of the types that are either:
   * - exposed by this Injector (i.e. in the Injector's type parameters)
   * - bound by a multibinding
   * - needed to inject one of the above (directly or indirectly)
   *
   * Unreachable bindings (i.e. bindings that are not exposed by this Injector, and that are not used by any reachable
   * binding) are not processed. Bindings that are only used lazily, using a Provider, are NOT eagerly injected.
   *
   * Also note that this guarantee doesn't apply to Providers.
   */
  FRUIT_DEPRECATED_DECLARATION(void eagerlyInjectAll());

private:
  using Check1 = typename fruit::impl::meta::CheckIfError<fruit::impl::meta::Eval<
      fruit::impl::meta::CheckNoRequiredTypesInInjectorArguments(fruit::impl::meta::Type<P>...)>>::type;
  // Force instantiation of Check1.
  static_assert(true || sizeof(Check1), "");

  using Comp = fruit::impl::meta::Eval<fruit::impl::meta::ConstructComponentImpl(fruit::impl::meta::Type<P>...)>;

  using Check2 = typename fruit::impl::meta::CheckIfError<Comp>::type;
  using VoidType = fruit::impl::meta::Type<void>;
  // Force instantiation of Check2.
  static_assert(true || sizeof(Check2), "");
  using Check3 = typename fruit::impl::meta::CheckIfError<fruit::impl::meta::Eval<fruit::impl::meta::If(
      fruit::impl::meta::Not(fruit::impl::meta::IsEmptySet(typename Comp::RsSuperset)),
      fruit::impl::meta::ConstructErrorWithArgVector(fruit::impl::InjectorWithRequirementsErrorTag,
                                                     fruit::impl::meta::SetToVector(typename Comp::RsSuperset)),
      VoidType)>>::type;
  // Force instantiation of Check3.
  static_assert(true || sizeof(Check3), "");

  friend struct fruit::impl::InjectorAccessorForTests;

  std::unique_ptr<fruit::impl::InjectorStorage> storage;
};

} // namespace fruit

#include <fruit/impl/injector.defn.h>

#endif // FRUIT_INJECTOR_H
