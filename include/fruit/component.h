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
 */
template <typename... Params>
class Component : public PartialComponent<fruit::impl::ConstructComponentImpl<Params...>> {
public:
  Component(Component&&) = default;
  Component(const Component&) = default;
  
  /**
   * Converts a PartialComponent (or Component) to an arbitrary Component, auto-injecting the missing types (if any).
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
 * from a function that has Component<...> as return type).
 * 
 * The template parameter is used to propagate information about bound types; users of the Fruit library can pretend that this
 * class is not templated, in no case a specific template parameter is required.
 */
template <typename Comp>
class PartialComponent {
private:
  template <typename Signature>
  using FunctionSignature = fruit::impl::FunctionSignature<Signature>;
  
  template <typename Functor>
  using ResultOf = typename Functor::Result;
  
public:
  PartialComponent(PartialComponent&&) = default;
  PartialComponent(const PartialComponent&) = default;

  /**
   * Converts a PartialComponent (or Component) to an arbitrary PartialComponent, auto-injecting the missing types (if any).
   */
  template <typename OtherComp>
  PartialComponent(PartialComponent<OtherComp> sourceComponent);
  
  /**
   * Binds the base class (typically, an interface or abstract class) I to the implementation C.
   */
  template <typename I, typename C>
  PartialComponent<ResultOf<fruit::impl::Bind<Comp, I, C>>> bind() &&;
  
  /**
   * Registers Signature as the constructor signature to use to inject a type.
   * For example, registerConstructor<C(U,V)>() registers the constructor C::C(U,V).
   * 
   * It's usually more convenient to use an Inject typedef or INJECT macro instead, e.g.:
   * 
   * class C {
   * public:
   *   // This also declares the constructor
   *   INJECT(C(U u, V v));
   * ...
   * };
   * 
   * or
   * 
   * class C {
   * public:
   *   using Inject = C(U,V);
   * ...
   * };
   * 
   * Use registerConstructor() when you want to inject the class C in different ways
   * in different components, or when C is a third-party class that can't be modified.
   */
  template <typename Signature>
  PartialComponent<ResultOf<fruit::impl::RegisterConstructor<Comp, Signature>>> registerConstructor() &&;
  
  /**
   * Use this method to bind the type C to a specific instance.
   * The caller must ensure that the provided reference is valid for the lifetime of this
   * component and of any injectors using this component, and must ensure that the object
   * is deleted after the last components/injectors using it are destroyed.
   * 
   * Returns a PartialComponent (usually with different type arguments).
   * 
   * This should be used sparingly, but in some cases it can be useful; for example, if
   * a web server creates an injector to handle each request, this method can be used
   * to inject the request itself.
   */
  template <typename C>
  PartialComponent<ResultOf<fruit::impl::RegisterInstance<Comp, C>>> bindInstance(C& instance) &&;
  
  /**
   * Registers `provider' as a provider of C, where provider is a function returning either C or C*
   * (prefer returning a C by value instead of allocating a C using `new C', if possible).
   * A lambda with no captures can be used as a function.
   * When an instance of C is needed, the arguments of the provider will be injected
   * and the provider will be called to create the instance of C, that will then be
   * stored in the injector.
   * `provider' must return a non-null pointer, otherwise the program will abort.
   * 
   * Returns a PartialComponent (usually with different type arguments).
   * 
   * Example:
   * 
   * registerProvider([](U* u, V* v) {
   *    C c(u, v);
   *    c.initialize();
   *    return std::move(c);
   * })
   * 
   * As in the previous example, it's not necessary to specify the signature, it will
   * be inferred by the compiler.
   * 
   * Registering stateful functors (including lambdas with captures) is not supported.
   * However, instead of registering a functor F to provide a C, it's possible to bind F
   * (binding an instance if necessary) and then use this method to register a provider
   * function that takes a F and any other needed parameters, calls F with such parameters
   * and returns a C*.
   */
  template <typename Function>
  PartialComponent<ResultOf<fruit::impl::RegisterProvider<Comp, FunctionSignature<Function>>>>
      registerProvider(Function provider) &&;
  
  /**
   * Similar to bind(), but adds a multibinding instead.
   * 
   * Note that multibindings are independent from bindings; creating a binding with bind doesn't count as a multibinding,
   * and adding a multibinding doesn't allow to inject the type (only allows to retrieve multibindings through the
   * getMultibindings method of the injector).
   * 
   * Returns a PartialComponent (with the same type arguments).
   */
  template <typename I, typename C>
  PartialComponent<ResultOf<fruit::impl::AddMultibinding<Comp, I, C>>> addMultibinding() &&;
  
  /**
   * Similar to bindInstance, but adds a multibinding instead.
   * 
   * Note that multibindings are independent from bindings; creating a binding with bindInstance doesn't count as a
   * multibinding, and adding a multibinding doesn't allow to inject the type (only allows to retrieve
   * multibindings through the getMultibindings method of the injector).
   * 
   * Returns a PartialComponent (with the same type arguments).
   */
  template <typename C>
  PartialComponent<ResultOf<fruit::impl::AddInstanceMultibinding<Comp, C>>> addInstanceMultibinding(C& instance) &&;
  
  /**
   * Similar to registerProvider, but adds a multibinding instead.
   * 
   * Note that multibindings are independent from bindings; creating a binding with registerProvider doesn't count as a
   * multibinding, and adding a multibinding doesn't allow to inject the type (only allows to retrieve multibindings
   * through the getMultibindings method of the injector).
   * 
   * Returns a PartialComponent (with the same type arguments).
   */
  template <typename Function>
  PartialComponent<ResultOf<fruit::impl::RegisterMultibindingProvider<Comp, FunctionSignature<Function>>>>
      addMultibindingProvider(Function provider) &&;
    
  /**
   * Registers `factory' as a factory of C, where `factory' is a function returning C.
   * A lambda with no captures can be used as a function.
   * C can be any class type; special support is provided if C is std::unique_ptr<T>.
   * 
   * Returns a PartialComponent (usually with different type arguments).
   * 
   * Example:
   * 
   * registerFactory<C(Assisted<U*>, V*)>([](U* u, V* v) {
   *   return new C(u, v);
   * })
   * 
   * As in the previous example, this is usually used for assisted injection. Unlike
   * registerProvider, where the signature is inferred, for this method the signature
   * must be specified explicitly.
   * This can be used for assisted injection: some parameters are marked as Assisted
   * and are not injected. Instead of calling injector.get<C*>(), in this example we will
   * call injector.get<std::function<C(U*)>(), or we will declare std::function<C(U*)> as
   * an injected parameter to another provider or class.
   * 
   * If the only thing that the factory does is to call the constructor of C, it's usually
   * more convenient to use an Inject typedef or INJECT macro instead, e.g.:
   * 
   * class C {
   * public:
   *   // This also declares the constructor
   *   INJECT(C(ASSISTED(U) u, V v));
   * ...
   * };
   * 
   * or
   * 
   * class C {
   * public:
   *   using Inject = C(Assisted<U>,V);
   * ...
   * };
   * 
   * Use registerFactory() when you want to inject the class C in different ways
   * in different components, or when C is a third-party class that can't be modified.
   * 
   * Registering stateful functors (including lambdas with captures) is not supported.
   * However, instead of registering a functor F to provide a C, it's possible to bind F
   * (binding an instance if necessary) and then use this method to register a provider
   * function that takes a F and any other needed parameters, calls F with such parameters
   * and returns a C*.
   */
  template <typename AnnotatedSignature>
  PartialComponent<ResultOf<fruit::impl::RegisterFactory<Comp, AnnotatedSignature>>>
      registerFactory(fruit::impl::RequiredSignatureForAssistedFactory<AnnotatedSignature>* factory) &&;
  
  /**
   * Adds the bindings in `component' to the current component.
   * 
   * Returns a PartialComponent (usually with different type arguments).
   * 
   * For example:
   * 
   * createComponent()
   *    .install(getComponent1())
   *    .install(getComponent2())
   *    .bind<I, C>()
   * 
   * As seen in the example, the template parameters will be inferred by the compiler,
   * it's not necessary to specify them explicitly.
   */
  template <typename... OtherCompParams>
  PartialComponent<ResultOf<fruit::impl::InstallComponent<Comp, fruit::impl::ConstructComponentImpl<OtherCompParams...>>>> 
      install(Component<OtherCompParams...> component) &&;
  
private:
  template <typename OtherComp>
  friend class PartialComponent;
  
  friend class Component<>;
  
  template <typename... Params>
  friend class NormalizedComponent;
  
  template <typename... Params>
  friend class Injector;
  
  fruit::impl::ComponentStorage storage;
  
  // Do not use. Use fruit::createComponent() instead.
  PartialComponent() = default;
  
  PartialComponent(fruit::impl::ComponentStorage&& storage);
};

} // namespace fruit

#include "impl/component.templates.h"


#endif // FRUIT_COMPONENT_H
