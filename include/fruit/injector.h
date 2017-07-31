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
#include <fruit/provider.h>
#include <fruit/normalized_component.h>

namespace fruit {

/**
 * An injector is a class constructed from a component that performs the needed injections and manages the lifetime of the created
 * objects.
 * An injector does *not* need to specify all types bound in the component; you can only specify the "root" type(s) and the
 * injector will also create and store the instances of classes that are needed (directly or indirectly) to inject the root types.
 * 
 * Example usage:
 * 
 * Injector<Foo, Bar> injector(getFooBarComponent());
 * Foo* foo = injector.get<Foo*>();
 * Bar* bar(injector); // Equivalent to: Bar* bar = injector.get<Bar*>();
 */
template <typename... P>
class Injector {
private:
  template <typename T>
  struct RemoveAnnotationsHelper {
    using type = fruit::impl::meta::UnwrapType<fruit::impl::meta::Eval<
        fruit::impl::meta::RemoveAnnotations(fruit::impl::meta::Type<T>)
        >>;
  };

  template <typename T>
  using RemoveAnnotations = typename RemoveAnnotationsHelper<T>::type;
  
public:
  // Moving injectors is allowed.
  Injector(Injector&&) = default;
  
  // Copying injectors is forbidden.
  Injector(const Injector&) = delete;
  
  /**
   * Creation of an injector from a component function (that can optionally have parameters).
   *
   * Example usage:
   *
   * Injector<Foo, Bar> injector(getFooBarComponent);
   * Foo* foo = injector.get<Foo*>();
   * Bar* bar(injector); // Equivalent to: Bar* bar = injector.get<Bar*>();
   */
  template <typename... FormalArgs, typename... Args>
  Injector(Component<P...>(*)(FormalArgs...), Args&&... args);
  
  /**
   * Creation of an injector from a normalized component and a component.
   * 
   * This constructor can be used to improve performance when creating many injectors that share the vast majority of the bindings
   * (or even all), by processing the common bindings in advance, and then adding the ones that differ.
   * If most bindings differ, the use of NormalizedComponent and this constructor won't improve performance; in that case, use the
   * single-argument constructor instead.
   * 
   * The NormalizedComponent can have requirements, but the Component can't.
   * The NormalizedComponent must remain valid during the lifetime of any Injector object constructed with it.
   * 
   * Note that a PartialComponent<...> can NOT be used as second argument, so if the second component is defined inline it must be
   * explicitly casted to the desired Component<...> type.
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
  template <typename... NormalizedComponentParams, typename... ComponentParams, typename... FormalArgs, typename... Args>
  Injector(const NormalizedComponent<NormalizedComponentParams...>& normalized_component,
           Component<ComponentParams...>(*)(FormalArgs...), Args&&... args);
  
  /**
   * Deleted constructor, to ensure that constructing an Injector from a temporary NormalizedComponent doesn't compile.
   * The NormalizedComponent must remain valid during the lifetime of any Injector object constructed with it.
   */
  template <typename... NormalizedComponentParams, typename... ComponentParams, typename... FormalArgs, typename... Args>
  Injector(NormalizedComponent<NormalizedComponentParams...>&& normalized_component, 
           Component<ComponentParams...>(*)(FormalArgs...), Args&&... args) = delete;
  
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
   * get<Provider<C>>()
   * get<Annotated<Annotation, C>>()             (for any type `Annotation')
   * get<Annotated<Annotation, C*>>()            (for any type `Annotation')
   * get<Annotated<Annotation, C&>>()            (for any type `Annotation')
   * get<Annotated<Annotation, const C*>>()      (for any type `Annotation')
   * get<Annotated<Annotation, const C&>>()      (for any type `Annotation')
   * get<Annotated<Annotation, shared_ptr<C>>>() (for any type `Annotation')
   * get<Annotated<Annotation, Provider<C>>>()   (for any type `Annotation')
   * 
   * With a non-annotated parameter T, this returns a T.
   * With an annotated parameter T=Annotated<Annotation, SomeClass>, this returns a SomeClass.
   * 
   * The shared_ptr versions come with a slight performance hit, avoid it if possible.
   * Calling get<> repeatedly for the same class with the same injector will return the same instance.
   */
  template <typename T>
  RemoveAnnotations<T> get();
  
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
   * Because foo would be of type fruit::Annotated, not of type SomeClass. In that case you must use get instead, e.g.:
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
   * With an annotated parameter T=Annotated<Annotation, SomeClass>, this returns a const std::vector<SomeClass*>&.
   */
  template <typename T>
  const std::vector<RemoveAnnotations<T>*>& getMultibindings();
  
  /**
   * Eagerly injects all reachable bindings and multibindings of this injector.
   * This only creates instances of the types that are either:
   * - exposed by this Injector (i.e. in the Injector's type parameters)
   * - all multibindings
   * - needed to inject one of the above (directly or indirectly)
   * 
   * Unreachable bindings (i.e. bindings that are not exposed by this Injector, and that are not used by any reachable binding)
   * are not processed. Bindings that are only used lazily, using a Provider, are NOT eagerly injected.
   * 
   * Call this to ensure thread safety if the injector will be shared by multiple threads.
   * After calling this method, get() and getMultibindings() can be called concurrently on the same injector, with no locking.
   * Note that the guarantee only applies after this method returns; specifically, this method can NOT be called concurrently
   * unless it has been called before on the same injector and returned.
   */
  void eagerlyInjectAll();
  
private:
  using Check1 = typename fruit::impl::meta::CheckIfError<fruit::impl::meta::Eval<
                      fruit::impl::meta::CheckNoRequiredTypesInInjectorArguments(fruit::impl::meta::Type<P>...)
                      >>::type;
  // Force instantiation of Check1.
  static_assert(true || sizeof(Check1), "");

  using Comp = fruit::impl::meta::Eval<fruit::impl::meta::ConstructComponentImpl(fruit::impl::meta::Type<P>...)>;

  using Check2 = typename fruit::impl::meta::CheckIfError<Comp>::type;
  using VoidType = fruit::impl::meta::Type<void>;
  // Force instantiation of Check2.
  static_assert(true || sizeof(Check2), "");
  using Check3 = typename fruit::impl::meta::CheckIfError<fruit::impl::meta::Eval<fruit::impl::meta::If(
                      fruit::impl::meta::Not(fruit::impl::meta::IsEmptySet(typename Comp::RsSuperset)),
                      fruit::impl::meta::ConstructErrorWithArgVector(fruit::impl::InjectorWithRequirementsErrorTag, fruit::impl::meta::SetToVector(typename Comp::RsSuperset)),
                      VoidType)
                      >>::type;
  // Force instantiation of Check3.
  static_assert(true || sizeof(Check3), "");

  friend struct fruit::impl::InjectorAccessorForTests;
  
  std::unique_ptr<fruit::impl::InjectorStorage> storage;
};

} // namespace fruit

#include <fruit/impl/injector.defn.h>

#endif // FRUIT_INJECTOR_H
