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

#ifndef FRUIT_COMPONENT_H
#define FRUIT_COMPONENT_H

#include "fruit_forward_decls.h"
#include "impl/component_impl.h"

namespace fruit {

/**
 * The parameters must be of the form <P...> or <Required<R...>, P...> where R... are the required types and P... are the
 * provided ones. If the list of requirements is empty it can be omitted.
 * No type can appear twice, not even once in R... and once in P....
 * 
 * See PartialComponent below for the methods available in this class.
 */
template <typename... Params>
class Component : public PartialComponent<fruit::impl::Apply<fruit::impl::ConstructComponentImpl, Params...>> {
public:
  Component(Component&&) = default;
  Component(const Component&) = default;
  
  Component& operator=(Component&&) = delete;
  Component& operator=(const Component&) = delete;
  
  /**
   * Converts a PartialComponent (or a Component) to an arbitrary Component, auto-injecting the missing types (if any).
   * This is usually called implicitly when returning a component from a function.
   */
  template <typename OtherComp>
  Component(PartialComponent<OtherComp> component);
  
private:
  // Do not use. Use fruit::createComponent() instead.
  Component() = default;
  
  friend Component<> createComponent();
};

/**
 * Constructs an empty component.
 */
Component<> createComponent();

/**
 * A partially constructed component.
 * 
 * Client code should never write `PartialComponent'; always start the construction of a component with fruit::createComponent(),
 * and end it by casting the PartialComponent to the desired Component (often done implicitly by returning a PartialComponent
 * from a function that has Component<...> as the return type).
 * 
 * The template parameter is used to propagate information about bound types, it is purely an implementation detail; users of the
 * Fruit library can pretend that this class is not templated, in no case a specific template parameter is required. All methods
 * of this class take `this' as &&, allowing to chain method invocations without declaring a variable of type PartialComponent.
 * 
 * Example usage:
 * 
 * fruit::Component<Foo> getFooComponent() {
 *   return fruit::createComponent()
 *      .install(getComponent1())
 *      .install(getComponent2())
 *      .bind<Foo, FooImpl>();
 * }
 * 
 * Note that no variable of type PartialComponent has been declared; this class should only be used for temporary values.
 */
template <typename Comp>
class PartialComponent {
private:
  template <typename Functor>
  using ResultOf = typename Functor::Result;
  
public:
  PartialComponent(PartialComponent&&) = default;
  PartialComponent(const PartialComponent&) = default;

  /**
   * Binds the base class (typically, an interface or abstract class) I to the implementation C.
   */
  template <typename I, typename C>
  PartialComponent<ResultOf<fruit::impl::Bind<Comp, I, C>>>
      bind() &&;
  
  /**
   * Registers Signature as the constructor signature to use to inject a type.
   * 
   * Example usage:
   * 
   * fruit::createComponent()
   *     .registerConstructor<Foo(Bar*,Baz*)>() // Registers the constructor Foo::Foo(Bar*,Baz*)
   * 
   * It's usually more convenient to use an INJECT macro or Inject typedef instead, e.g.:
   * 
   * class Foo {
   * public:
   *   // This also declares the constructor
   *   INJECT(Foo(Bar* bar, Baz* baz));
   * ...
   * };
   * 
   * or (equivalent):
   * 
   * class Foo {
   * public:
   *   using Inject = Foo(Bar*, Baz*);
   *   Foo(Bar* bar, Baz* baz);
   * ...
   * };
   * 
   * Use registerConstructor() when you want to inject the class C in different ways in different components (just make sure those
   * don't end up in the same injector), or when C is a third-party class that can't be modified.
   */
  template <typename Signature>
  PartialComponent<ResultOf<fruit::impl::RegisterConstructor<Comp, Signature>>>
      registerConstructor() &&;
  
  /**
   * Use this method to bind the type C to a specific instance.
   * The caller must ensure that the provided reference is valid for the entire lifetime of the component and of any components
   * or injectors that install this component; the caller must also ensure that the object is destroyed after the last
   * components/injectors using it are destroyed.
   * 
   * Example usage:
   * 
   * NormalizedComponent<...> normalizedComponent = ...;
   * Request request;
   * Injector<...> injector(normalizedComponent,
   *                        Component<Request>(fruit::createComponent()
   *                            .bindInstance(request)));
   * 
   * This should be used sparingly (let Fruit handle the object lifetime when possible), but in some cases it is necessary; for
   * example, if a web server creates an injector to handle each request, this method can be used to inject the request itself.
   */
  template <typename C>
  PartialComponent<ResultOf<fruit::impl::RegisterInstance<Comp, C>>>
      bindInstance(C& instance) &&;
  
  /**
   * Registers `provider' as a provider of C, where provider is a lambda with no captures returning either C or C* (prefer
   * returning a C by value instead of allocating a C using `new C', to avoid the allocation).
   * 
   * When injecting a C, the arguments of the provider will be injected and the provider will then be called to create the
   * C instance, that will then be stored in the injector.
   * 
   * If `provider' returns a pointer, it must be non-null; otherwise the program will abort.
   * 
   * Example:
   * 
   * registerProvider([](Bar* bar, Baz* baz) {
   *    Foo foo(bar, baz);
   *    foo.initialize();
   *    return std::move(foo);
   * })
   * 
   * As in the previous example, it's not necessary to specify the type parameter, it will be inferred by the compiler.
   * 
   * registerProvider() can't be called with a plain function, but you can write a lambda that wraps the function to achieve the
   * same result.
   * 
   * Registering a stateful functors (including lambdas with captures) is NOT supported.
   * However, instead of registering a functor F to provide a class C, it's possible to bind F (binding an instance if necessary)
   * and to then use this registerProvider to register a provider that takes a F and any other needed parameters, calls F with
   * such parameters and returns a C (or C*).
   */
  template <typename Provider>
  PartialComponent<ResultOf<fruit::impl::RegisterProvider<Comp, Provider>>>
      registerProvider(Provider provider) &&;
  
  /**
   * Similar to bind<I, C>(), but adds a multibinding instead.
   * 
   * Multibindings are independent from bindings; creating a binding with bind doesn't count as a multibinding, and adding a
   * multibinding doesn't allow to inject the type (it only allows to retrieve multibindings through the getMultibindings method
   * of the injector).
   * 
   * Unlike bindings, where adding a the same binding twice is allowed (and ignored), adding the same multibinding multiple times
   * will result in the creation of multiple "equivalent" instances, that will all be returned by getMultibindings().
   * It is good practice to add the multibindings in a component that is "close" to the injector, to avoid installing that
   * component more than once.
   */
  template <typename I, typename C>
  PartialComponent<ResultOf<fruit::impl::AddMultibinding<Comp, I, C>>>
      addMultibinding() &&;
  
  /**
   * Similar to bindInstance(), but adds a multibinding instead.
   * 
   * Multibindings are independent from bindings; creating a binding with bindInstance doesn't count as a
   * multibinding, and adding a multibinding doesn't allow to inject the type (only allows to retrieve
   * multibindings through the getMultibindings method of the injector).
   * 
   * Unlike bindings, where adding a the same binding twice is allowed (and ignored), adding several multibindings for the same
   * instance will result in duplicated values in the result of getMultibindings.
   * It is good practice to add the multibindings in a component that is "close" to the injector, to avoid installing that
   * component more than once.
   */
  template <typename C>
  PartialComponent<ResultOf<fruit::impl::AddInstanceMultibinding<Comp, C>>>
      addInstanceMultibinding(C& instance) &&;
  
  /**
   * Similar to registerProvider, but adds a multibinding instead.
   * 
   * Multibindings are independent from bindings; creating a binding with registerProvider doesn't count as a
   * multibinding, and adding a multibinding doesn't allow to inject the type (only allows to retrieve multibindings
   * through the getMultibindings method of the injector).
   * 
   * Unlike bindings, where adding a the same binding twice is allowed (and ignored), adding the same multibinding provider
   * multiple times will result in the creation of multiple "equivalent" instances, that will all be returned by getMultibindings.
   * It is good practice to add the multibindings in a component that is "close" to the injector, to avoid installing that
   * component more than once.
   */
  template <typename Provider>
  PartialComponent<ResultOf<fruit::impl::RegisterMultibindingProvider<Comp, Provider>>>
      addMultibindingProvider(Provider provider) &&;
    
  /**
   * Registers `factory' as a factory of C, where `factory' is a lambda with no captures returning C.
   * 
   * C can be any class type. If C is std::unique_ptr<T>, the factory together with a bind<I,C> in the same component
   * will automatically bind the corresponding std::function that returns a std::unique_ptr<I>.
   * 
   * C can NOT be a pointer type. If you don't want to return by value, return a std::unique_ptr instead of a naked pointer.
   * 
   * Example:
   * 
   * fruit::createComponent()
   *     .registerFactory<std::unique_ptr<Foo>(Assisted<int>, Bar*)>(
   *        [](int n, Bar* bar) {
   *            return std::unique_ptr<Foo>(new Foo(n, bar));
   *        })
   * 
   * Unlike registerProvider(), where the signature is inferred, for this method the signature (including any Assisted
   * annotations) must be specified explicitly, while the second template parameter is inferred.
   * 
   * This can be used for assisted injection: some parameters are marked as Assisted and are not injected. Instead of calling
   * injector.get<C*>(), in this example we will call injector.get<std::function<std::unique_ptr<Foo>(int)>(), or we will declare
   * std::function<std::unique_ptr<Foo>(int)> as an injected parameter to another provider or class.
   * 
   * If the only thing that the factory does is to call the constructor of the class, it's usually more convenient to use an
   * Inject typedef or INJECT macro instead, e.g. the following are equivalent to the above:
   * 
   * class Foo {
   * public:
   *   // This also declares the constructor
   *   INJECT(Foo(ASSISTED(int) n, Bar* bar));
   * ...
   * };
   * 
   * or
   * 
   * class Foo {
   * public:
   *   using Inject = Foo(Assisted<int> n, Bar* bar);
   *   Foo(int n, Bar* bar);
   * ...
   * };
   * 
   * Use registerFactory() when you want to inject the class C in different ways in different components (just make sure those
   * don't end up in the same injector), or when C is a third-party class that can't be modified.
   * 
   * registerFactory() can't be called with a plain function, but you can write a lambda that wraps the function to achieve the
   * same result.
   * 
   * Registering stateful functors (including lambdas with captures) is NOT supported.
   * However, instead of registering a functor F to provide a class C, it's possible to bind F (binding an instance if necessary)
   * and to then use this method to register a provider that takes a F and any other needed parameters, calls F with such
   * parameters and then returns a C (or C*).
   */
  template <typename AnnotatedSignature, typename Factory>
  PartialComponent<ResultOf<fruit::impl::RegisterFactory<Comp, AnnotatedSignature, Factory>>>
      registerFactory(Factory factory) &&;
  
  /**
   * Adds the bindings (and multibindings) in `component' to the current component.
   * 
   * Example usage:
   * 
   * createComponent()
   *    .install(getComponent1())
   * 
   * As seen in the example, the template parameters will be inferred by the compiler, it's not necessary to specify them
   * explicitly.
   */
  template <typename... OtherCompParams>
  PartialComponent<ResultOf<fruit::impl::InstallComponent<Comp, fruit::impl::Apply<fruit::impl::ConstructComponentImpl, OtherCompParams...>>>> 
      install(Component<OtherCompParams...> component) &&;
  
private:
  template <typename OtherComp>
  friend class PartialComponent;
  
  template <typename... Types>
  friend class Component;
  
  template <typename... Params>
  friend class NormalizedComponent;
  
  template <typename... Params>
  friend class Injector;
  
  fruit::impl::ComponentStorage storage;
  
  // Do not use. Use fruit::createComponent() instead.
  PartialComponent() = default;
  
  PartialComponent(fruit::impl::ComponentStorage&& storage);
  
  /**
   * Converts a PartialComponent (or Component) to an arbitrary PartialComponent, auto-injecting the missing types (if any).
   * For internal usage only. Users of the library should use the similar constructor in Component instead.
   */
  template <typename OtherComp>
  PartialComponent(PartialComponent<OtherComp> sourceComponent);
};

} // namespace fruit

#include "impl/component.defn.h"


#endif // FRUIT_COMPONENT_H
