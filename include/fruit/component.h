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

#ifndef FRUIT_MODULE_H
#define FRUIT_MODULE_H

#include "impl/component_storage.h"
#include "impl/component.utils.h"
#include "impl/injection_errors.h"
#include "impl/basic_utils.h"
#include "impl/fruit_assert.h"

namespace fruit {

namespace impl {

template <typename M>
struct Identity;

template <typename M, typename I, typename C>
struct Bind;

template <typename M, typename Signature>
struct RegisterProvider;

template <typename M, typename AnnotatedSignature>
struct RegisterFactory;

template <typename M, typename C>
struct RegisterInstance;

template <typename M, typename Signature>
struct RegisterConstructor;

template <typename M, typename AnnotatedSignature>
struct RegisterConstructorAsFactory;

template <typename M, typename OtherM>
struct InstallComponent;

template <typename M, typename ToRegister>
struct ComponentConversionHelper;

template <typename M, typename TargetRequirements, bool is_already_provided, typename C>
struct EnsureProvidedTypeHelper;

/**
 * This class contains the methods of Component (which will be used by this library's users) but should not
 * be instantiated directly by client code. I.e. a user of the library should never write `ComponentImpl'.
 * Always start the construction of a component with createComponent().
 */
template <typename RsParam, typename PsParam, typename DepsParam>
struct ComponentImpl {
public:
  // Just for convenience.
  using Rs = RsParam;
  using Ps = PsParam;
  using Deps = DepsParam;
  using M = ComponentImpl<Rs, Ps, Deps>;
  
  // Invariants:
  // * all types appearing as arguments of Deps are in Rs
  // * all types in Ps are at the head of one (and only one) Dep.
  //   (note that the types in Rs can appear in deps any number of times, 0 is also ok)
  // * Deps is of the form List<Dep...> with each Dep of the form T(Args...) and where List<Args...> is a set (no repetitions).
  
private:
  FruitStaticAssert(std::is_same<AddDeps<Deps, List<>>, Deps>::value,
                    "Internal error: ComponentImpl instantiated with non-normalized deps");
    
  // Invariant: all types in Ps must be bound in unsafeComponent.
  ComponentStorage unsafeComponent;
  
  ComponentImpl() = default;
  
  ComponentImpl(ComponentStorage&& unsafeComponent);
    
  template <typename... Types>
  friend class fruit::Injector;
  
  template <typename... Types>
  friend class fruit::Component;
  
  template <typename OtherRs, typename OtherPs, typename OtherDeps>
  friend struct fruit::impl::ComponentImpl;
  
  template <typename M, typename ToRegister>
  friend struct fruit::impl::ComponentConversionHelper;
  
  template <typename M, typename TargetRequirements, bool is_already_provided, typename C>
  friend struct fruit::impl::EnsureProvidedTypeHelper;
    
  template <typename M>
  friend struct fruit::impl::Identity;
  
  template <typename M, typename I, typename C>
  friend struct fruit::impl::Bind;
  
  template <typename M, typename C>
  friend struct fruit::impl::RegisterInstance;
  
  template <typename M, typename Signature>
  friend struct fruit::impl::RegisterProvider;
  
  template <typename M, typename AnnotatedSignature>
  friend struct fruit::impl::RegisterFactory;
  
  template <typename M, typename Signature>
  friend struct RegisterConstructor;
  
  template <typename M, typename AnnotatedSignature>
  friend struct fruit::impl::RegisterConstructorAsFactory;
  
  template <typename M, typename OtherM>
  friend struct fruit::impl::InstallComponent;
  
  template <typename Source_Rs, typename Source_Ps, typename Source_Deps>
  ComponentImpl(const ComponentImpl<Source_Rs, Source_Ps, Source_Deps>& sourceComponent);
  
public:
  /**
   * Binds the base class (typically, an interface or abstract class) I to the implementation C.
   */
  template <typename I, typename C>
  FunctorResult<Bind<M, I, C>, M&&>
  bind() && {
    return Bind<M, I, C>()(std::move(*this));
  }
  
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
  FunctorResult<RegisterConstructor<M, Signature>, M&&>
  registerConstructor() && {
    return RegisterConstructor<M, Signature>()(std::move(*this));
  }
  
  /**
   * Use this to bind the type C to a specific instance.
   * The caller must ensure that the provided pointer is valid for the lifetime of this
   * component and of any injectors using this component, and must ensure that the object
   * is deleted after the last components/injectors using it are destroyed.
   * 
   * This should be used sparingly, but in some cases it can be useful; for example, if
   * a web server creates an injector to handle each request, this method can be used
   * to inject the request itself.
   */
  template <typename C>
  FunctorResult<RegisterInstance<M, C>, M&&, C*>
  bindInstance(C* instance) && {
    return RegisterInstance<M, C>()(std::move(*this), instance);
  }
  
  /**
   * Registers `provider' as a provider of C, where provider is a function
   * returning either C or C* (returning C* is preferable).
   * When an instance of C is needed, the arguments of the provider will be injected
   * and the provider will be called to create the instance of C, that will then be
   * stored in the injector.
   * `provider' must return a non-null pointer, otherwise the program will abort.
   * Example:
   * 
   * registerProvider([](U* u, V* v) {
   *    C* c = new C(u, v);
   *    c->initialize();
   *    return c;
   * })
   * 
   * As in the previous example, it's usually not necessary to specify the signature,
   * it will be inferred by the compiler.
   */
  template <typename Signature>
  FunctorResult<RegisterProvider<M, Signature>, M&&, Signature*>
  registerProvider(Signature* provider) && {
    return RegisterProvider<M, Signature>()(std::move(*this), provider);
  }
  
  /**
   * Registers `factory' as a factory of C, where `factory' is a function
   * returning either C or C* (returning C* is preferable)
   * 
   * registerFactory<C(Assisted<U*>, V*)>([](U* u, V* v) {
   *   return new C(u, v);
   * })
   * 
   * As in the previous example, this is usually used for assisted injection. Unlike
   * registerProvider, where the signature is inferred, for this method the signature
   * must be specified explicitly.
   * This is usually used for assisted injection: some parameters are marked as Assisted
   * and are not injected. Instead of calling injector.get<C*>(), in this example we will
   * call injector.get<std::function<C(U*)>() (or we will declare std::function<C(U*)> as
   * an injected parameter to another provider or class).
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
   */
  template <typename AnnotatedSignature>
  FunctorResult<RegisterFactory<M, AnnotatedSignature>, M&&, RequiredSignatureForAssistedFactory<AnnotatedSignature>*>
  registerFactory(RequiredSignatureForAssistedFactory<AnnotatedSignature>* factory) && {
    return RegisterFactory<M, AnnotatedSignature>()(std::move(*this), factory);
  }
  
  /**
   * Adds the bindings in `component' to the current component.
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
  template <typename OtherRs, typename OtherPs, typename OtherDeps>
  FunctorResult<InstallComponent<M, ComponentImpl<OtherRs, OtherPs, OtherDeps>>, M&&, const ComponentImpl<OtherRs, OtherPs, OtherDeps>&>
  install(const ComponentImpl<OtherRs, OtherPs, OtherDeps>& component) && {
    return InstallComponent<M, ComponentImpl<OtherRs, OtherPs, OtherDeps>>()(std::move(*this), component);
  }
};

} // namespace impl

// Used to group the requirements of Component.
template <typename... Types>
struct Required {};

// Used to annotate T as a type that uses assisted injection.
template <typename T>
struct Assisted;

/**
 * The parameters must be of the form <Required<R...>, P...> where R are the required types and P are the provided ones.
 * If the list of requirements is empty it can be omitted (e.g. Component<Foo, Bar>).
 * No type can appear twice. Not even once in R and once in P.
 */
template <typename... Types>
class Component;

template <typename... Types>
class Component : public Component<Required<>, Types...> {
private:
  Component() = default;
  
  friend Component<> createComponent();
  
public:
  /**
   * Converts a component to another, auto-injecting the missing types (if any).
   * This is typically called implicitly when returning a component from a function.
   * 
   * To copy a component, the most convenient way is to call createComponent().install(m).
   */
  template <typename M>
  Component(M&& m) 
    : Component<Required<>, Types...>(std::forward<M>(m)) {
  }
};

template <typename... R, typename... P>
class Component<Required<R...>, P...> 
  : public fruit::impl::ComponentImpl<fruit::impl::List<R...>,
                                   fruit::impl::List<P...>,
                                   fruit::impl::ConstructDeps<fruit::impl::List<R...>, P...>> {
private:
  FruitDelegateCheck(fruit::impl::CheckNoRepeatedTypes<R..., P...>);
  FruitDelegateChecks(fruit::impl::CheckClassType<R, fruit::impl::GetClassForType<R>>);  
  FruitDelegateChecks(fruit::impl::CheckClassType<P, fruit::impl::GetClassForType<P>>);  
  
  using Impl = fruit::impl::ComponentImpl<fruit::impl::List<R...>,
                                       fruit::impl::List<P...>,
                                       fruit::impl::ConstructDeps<fruit::impl::List<R...>, P...>>;
  
  Component() = default;
  
  template <typename... Types>
  friend class Component;
  
public:
  template <typename OtherRs, typename OtherPs, typename OtherDeps>
  Component(fruit::impl::ComponentImpl<OtherRs, OtherPs, OtherDeps>&& m) 
    : Impl(std::move(m)) {
  }
};

inline Component<> createComponent() {
  return {};
}


} // namespace fruit

#include "impl/component.templates.h"


#endif // FRUIT_MODULE_H
