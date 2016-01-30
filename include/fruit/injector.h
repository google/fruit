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
  using RemoveAnnotations = fruit::impl::meta::UnwrapType<fruit::impl::meta::Eval<
      fruit::impl::meta::RemoveAnnotations(fruit::impl::meta::Type<T>)
      >>;
  
public:
  // Moving injectors is allowed.
  Injector(Injector&&) = default;
  
  // Copying injectors is forbidden.
  Injector(const Injector&) = delete;
  
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
   * The NormalizedComponent must remain valid during the lifetime of any Injector object constructed with it.
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
   *   Foo* foo = injector.get<Foo*>();
   *   ...
   * }
   */
  template <typename... NormalizedComponentParams, typename... ComponentParams>
  Injector(const NormalizedComponent<NormalizedComponentParams...>& normalized_component, Component<ComponentParams...> component);
  
  /**
   * Deleted constructor, to ensure that constructing an Injector from a temporary NormalizedComponent doesn't compile.
   * The NormalizedComponent must remain valid during the lifetime of any Injector object constructed with it.
   */
  template <typename... NormalizedComponentParams, typename... ComponentParams>
  Injector(NormalizedComponent<NormalizedComponentParams...>&& normalized_component, 
           Component<ComponentParams...> component) = delete;
  
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
   * If C was bound (directly or indirectly) in the component used to create this injector, returns a pointer to the instance of C
   * (constructing it if necessary). Otherwise returns nullptr.
   * 
   * This supports annotated injection, just use Annotated<Annotation, C> instead of just C.
   * With a non-annotated parameter C, this returns a C*.
   * With an annotated parameter C=Annotated<Annotation, SomeClass>, this returns a const SomeClass*.
   * 
   * WARNING: Unlike get(), this method does not check that C is provided by this injector. In production code, always use get(),
   * so that you are guaranteed to catch missing bindings at compile time. This method might be useful in tests, since the
   * types exposed by each component are determined by the needs of the production code, not of the test code.
   * Also, this is slightly slower than get(), but as long as it's only used in test code the difference should not be noticeable.
   * 
   * Note that this doesn't trigger auto-bindings: so even if the constructor of C was visible to some get*Component function (or
   * to the place where unsafeGet is called), in order to successfully get an instance with this method you need all the
   * following to be true:
   * * C was explicitly bound in a component, or C was a dependency (direct or indirect) of a type that was explicitly bound
   * * C was not bound to any interface (note however that if C was bound to I, you can do unsafeGet<I>() instead).
   * Otherwise this method will return nullptr.
   * 
   * WARNING: This method depends on what types are bound internally. It's not too unlikely that the internal bindings might
   * change in a future version of Fruit (it already happened between 1.0 and 1.1). If this happens, it will be in the release
   * notes, and if you used this method you'll have to check that the existing uses still work.
   */
  template <typename C>
  RemoveAnnotations<C>* unsafeGet();
  
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
  using Comp = fruit::impl::meta::Eval<fruit::impl::meta::ConstructComponentImpl(fruit::impl::meta::Type<P>...)>;

  using Check1 = typename fruit::impl::meta::CheckIfError<Comp>::type;
  // Force instantiation of Check1.
  static_assert(true || sizeof(Check1), "");
  using Check2 = typename fruit::impl::meta::CheckIfError<fruit::impl::meta::Eval<fruit::impl::meta::If(
                      fruit::impl::meta::Not(fruit::impl::meta::IsEmptySet(typename Comp::RsSuperset)),
                      fruit::impl::meta::ConstructErrorWithArgVector(fruit::impl::InjectorWithRequirementsErrorTag, fruit::impl::meta::SetToVector(typename Comp::RsSuperset)),
                      fruit::impl::meta::Type<void>)
                      >>::type;
  // Force instantiation of Check2.
  static_assert(true || sizeof(Check2), "");
  
  std::unique_ptr<fruit::impl::InjectorStorage> storage;
};

} // namespace fruit

#include <fruit/impl/injector.defn.h>

#endif // FRUIT_INJECTOR_H
