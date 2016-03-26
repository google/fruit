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

// This include is not required here, but having it here shortens the include trace in error messages.
#include <fruit/impl/injection_errors.h>

#include <fruit/fruit_forward_decls.h>
#include <fruit/impl/bindings.h>
#include <fruit/impl/meta/component.h>
#include <fruit/impl/storage/component_storage.h>
#include <fruit/impl/component_functors.defn.h>

namespace fruit {

/**
 * The parameters can be of the form <P...> or <Required<R...>, P...> where:
 * * R... are the required types (types required to inject some types in P... but that are not provided by this Component), if any
 * * P... are the types provided by this Component.
 * No type can appear twice, not even once in R and once in P.
 * 
 * See PartialComponent below for the methods available in this class.
 */
template <typename... Params>
class Component {
public:
  Component(Component&&) = default;
  Component(const Component&) = default;
  
  Component& operator=(Component&&) = delete;
  Component& operator=(const Component&) = delete;
  
  /**
   * Converts a PartialComponent to an arbitrary Component, auto-injecting the missing types (if
   * any).
   * This is usually called implicitly when returning a component from a function. See
   * PartialComponent for an example.
   */
  template <typename... Bindings>
  Component(PartialComponent<Bindings...> component);
  
  /**
   * Conversion from an arbitrary Component.
   * This is equivalent to the component returned by:
   * Component<Params...>(createComponent().install(component))
   * It's provided only for convenience.
   */
  template <typename... OtherParams>
  Component(Component<OtherParams...> component);
  
private:
  // Do not use. Use fruit::createComponent() instead.
  Component() = default;
    
  template <typename... Bindings>
  friend class PartialComponent;

  template <typename... OtherParams>
  friend class NormalizedComponent;
  
  template <typename... OtherParams>
  friend class Injector;
  
  fruit::impl::ComponentStorage storage;
  
  using Comp = fruit::impl::meta::Eval<fruit::impl::meta::ConstructComponentImpl(fruit::impl::meta::Type<Params>...)>;

  using Check1 = typename fruit::impl::meta::CheckIfError<Comp>::type;
  // Force instantiation of Check1.
  static_assert(true || sizeof(Check1), "");
};

/**
 * Constructs an empty component.
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
 * Since types are auto-injected when needed, just converting this to the desired component can suffice in some cases, e.g.:
 * 
 * fruit::Component<Foo> getFooComponent() {
 *   return fruit::createComponent();
 * }
 * 
 * This works if Foo has an Inject typedef or a constructor wrapped in INJECT.
 */
PartialComponent<> createComponent();

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
template <typename... Bindings>
class PartialComponent {
public:
  PartialComponent(PartialComponent&&) = default;

  PartialComponent& operator=(PartialComponent&&) = delete;
  PartialComponent& operator=(const PartialComponent&) = delete;
  
  /**
   * Binds the base class (typically, an interface or abstract class) I to the implementation C.
   * 
   * This supports annotated injection, just wrap I and/or C in fruit::Annotated<> if desired.
   */
  template <typename I, typename C>
  PartialComponent<fruit::impl::Bind<I, C>, Bindings...> bind() &&;
  
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
   * 
   * This supports annotated injection, just wrap the desired types (return type and/or argument types of the signature)
   * with fruit::Annotated<> if desired.
   */
  template <typename Signature>
  PartialComponent<fruit::impl::RegisterConstructor<Signature>, Bindings...> registerConstructor() &&;
  
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
  PartialComponent<fruit::impl::BindInstance<C>, Bindings...> bindInstance(C& instance) &&;
  
  /**
   * Similar to the previous version of bindInstance(), but allows to specify an annotated type that
   * will be bound to the specified value.
   * For example, to bind an instance to the type Annotated<Hostname, std::string>, you can use:
   * 
   * fruit::createComponent()
   *     .bindInstance<fruit::Annotated<Hostname, std::string>>(hostname)
   */
  template <typename AnnotatedType, typename C>
  PartialComponent<fruit::impl::BindInstance<AnnotatedType>, Bindings...> bindInstance(C& instance) &&;
  
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
   * However, you can write something like: 
   * 
   * struct Functor {
   *   Functor(int n) {...}
   *   MyClass operator()(Foo* foo) {...}
   * };
   * 
   * Component<MyClass> getMyClassComponent() {
   *   static const Functor aFunctor(42);
   *   return fruit::createComponent()
   *       ... // Bind Foo
   *       .bindInstance(aFunctor)
   *       .registerProvider([](Functor functor, Foo* foo) { return functor(foo); });
   * }
   */
  template <typename Lambda>
  PartialComponent<fruit::impl::RegisterProvider<Lambda>, Bindings...> registerProvider(Lambda lambda) &&;

  /**
   * Similar to the previous version of registerProvider(), but allows to specify an annotated type
   * for the provider. This allows to inject annotated types in the parameters and/or bind the
   * provider to an annotated type. For example:
   * 
   * .registerProvider<Annotated<MyAnnotation, Foo>(Annotated<SomeOtherAnnotation, Bar*>, Baz*)>(
   *    [](Bar* bar, Baz* baz) {
   *      Foo foo(bar, baz);
   *      foo.initialize();
   *      return std::move(foo);
   *    })
   * 
   * Binds the type Foo (annotated with MyAnnotation) and injects the Bar annotated with
   * SomeOtherAnnotation as the first parameter of the lambda.
   */
  template <typename AnnotatedSignature, typename Lambda>
  PartialComponent<fruit::impl::RegisterProvider<AnnotatedSignature, Lambda>, Bindings...> registerProvider(Lambda lambda) &&;
  
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
   * 
   * This supports annotated injection, just wrap I and/or C in fruit::Annotated<> if desired.
   */
  template <typename I, typename C>
  PartialComponent<fruit::impl::AddMultibinding<I, C>, Bindings...> addMultibinding() &&;
  
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
   * 
   * This method adds a multibinding for C. If the object implements an interface I and you want to add a multibinding
   * for that interface instead, cast the object to I& before calling this.
   * 
   * Note that this takes the instance by reference, not by value; it must remain valid for the entire lifetime of this component
   * and of any injectors created from this component.
   */
  template <typename C>
  PartialComponent<Bindings...> addInstanceMultibinding(C& instance) &&;
  
  /**
   * Similar to the previous version of addInstanceMultibinding(), but allows to specify an
   * annotated type.
   * Example use:
   * 
   * createComponent()
   *     // With someObject of type MyClass
   *     .addInstanceMultibinding<Annotated<MyAnnotation, MyClass>>(someObject)
   */
  template <typename AnnotatedC, typename C>
  PartialComponent<Bindings...> addInstanceMultibinding(C& instance) &&;

  /**
   * Equivalent to calling addInstanceMultibinding() for each elements of `instances'.
   * See the documentation of addInstanceMultibinding() for more details.
   * 
   * Note that this takes the vector by reference, not by value; the vector (and its elements) must remain valid for the entire
   * lifetime of this component and of any injectors created from this component.
   */
  template <typename C>
  PartialComponent<Bindings...> addInstanceMultibindings(std::vector<C>& instances) &&;
  
  /**
   * Similar to the previous version of addInstanceMultibindings(), but allows to specify an
   * annotated type.
   * Example use:
   * 
   * createComponent()
   *     // With v of type std::vector<MyClass>
   *     .addInstanceMultibindings<Annotated<MyAnnotation, MyClass>>(v)
   */
  template <typename AnnotatedC, typename C>
  PartialComponent<Bindings...> addInstanceMultibindings(std::vector<C>& instance) &&;

  /**
   * Similar to registerProvider, but adds a multibinding instead.
   * 
   * Multibindings are independent from bindings; creating a binding with registerProvider doesn't count as a
   * multibinding, and adding a multibinding doesn't allow to inject the type (only allows to retrieve multibindings
   * through the getMultibindings method of the injector).
   * 
   * Unlike bindings, where adding a the same binding twice is allowed (and ignored), adding the same multibinding provider
   * multiple times will result in the creation of multiple "equivalent" instances, that will all be returned by getMultibindings.
   * It is good practice to add the multibindings in a component that is "close" to the injector in the get*Component call chain,
   * to avoid adding the same multibinding more than once.
   * 
   * Note that this method adds a multibinding for the type returned by the provider. If the returned object implements an
   * interface I and you want to add a multibinding for that interface instead, return a pointer casted to I*.
   */
  template <typename Lambda>
  PartialComponent<fruit::impl::AddMultibindingProvider<Lambda>, Bindings...> addMultibindingProvider(Lambda lambda) &&;
      
  /**
   * Similar to the previous version of addMultibindingProvider(), but allows to specify an annotated type
   * for the provider. This allows to inject annotated types in the parameters and/or bind the
   * provider to an annotated type. For example:
   * 
   * .addMultibindingProvider<Annotated<MyAnnotation, Foo>(Annotated<SomeOtherAnnotation, Bar*>, Baz*)>(
   *    [](Bar* bar, Baz* baz) {
   *      Foo foo(bar, baz);
   *      foo.initialize();
   *      return std::move(foo);
   *    })
   * 
   * Add a multibinding for the type Foo (annotated with MyAnnotation) and injects the Bar annotated with
   * SomeOtherAnnotation as the first parameter of the lambda.
   */
  template <typename AnnotatedSignature, typename Lambda>
  PartialComponent<fruit::impl::AddMultibindingProvider<AnnotatedSignature, Lambda>, Bindings...> addMultibindingProvider(Lambda lambda) &&;
    
  /**
   * Registers `factory' as a factory of C, where `factory' is a lambda with no captures returning C.
   * This is typically used for assisted injection (but can also be used if no parameters are assisted).
   * 
   * C can be any class type. If C is std::unique_ptr<T>, the factory together with a bind<I,C> in the same component
   * will automatically bind the corresponding std::function that returns a std::unique_ptr<I>.
   * 
   * C can NOT be a pointer type. If you don't want to return by value, return a std::unique_ptr instead of a naked pointer.
   * 
   * Example:
   * 
   * Component<std::function<std::unique_ptr<MyClass>(int)>> getMyClassComponent() {
   *   fruit::createComponent()
   *       ... // Bind Foo
   *       .registerFactory<std::unique_ptr<MyClass>(Foo*, Assisted<int>)>(
   *          [](Foo* foo, int n) {
   *              return std::unique_ptr<MyClass>(new MyClass(foo, n));
   *          });
   * }
   * 
   * and then, e.g. in main():
   * 
   * Injector<std::function<std::unique_ptr<MyClass>(int)>> injector(getMyClassComponent());
   * 
   * std::function<std::unique_ptr<MyClass>(int)> factory(injector);
   * std::unique_ptr<MyClass> x = factory(42);
   * 
   * Note that non-assisted parameters will be passed automatically by Fruit.
   * 
   * Unlike registerProvider(), where the signature is inferred, for this method the signature (including any Assisted
   * annotations) must be specified explicitly, while the second template parameter is inferred.
   * 
   * If the only thing that the factory does is to call new and the constructor of the class, it's usually more convenient to use
   * an Inject typedef or INJECT macro instead, e.g. the following are equivalent to the above:
   * 
   * class MyClass {
   * public:
   *    using Inject = MyClass(Foo*, Assisted<int>);
   * 
   *    MyClass(Foo* foo, int n) {...}
   * };
   * 
   * or:
   * 
   * class MyClass {
   * public:
   *    INJECT(MyClass(Foo* foo, ASSISTED(int) n) {...}
   * };
   * 
   * Use registerFactory() when you want to inject the class in different ways in different components (just make sure those
   * don't end up in the same injector), or when MyClass is a third-party class that can't be modified.
   * 
   * registerFactory() can't be called with a plain function, but you can write a lambda that wraps the function to achieve the
   * same result.
   * 
   * Registering stateful functors (including lambdas with captures) is NOT supported.
   * However, you can write something like: 
   * 
   * struct Functor {
   *   Functor(float x) {...}
   *   std::unique_ptr<MyClass> operator()(Foo* foo, int n) {...}
   * };
   * 
   * Component<std::function<std::unique_ptr<MyClass>(int)>> getMyClassComponent() {
   *   static const Functor aFunctor(42.0);
   *   return fruit::createComponent()
   *       ... // Bind Foo
   *       .bindInstance(aFunctor)
   *       .registerFactory<std::unique_ptr<MyClass>(Functor functor, Foo*, Assisted<int>)>(
   *            [](Functor* functor, Foo* foo, int n) { return functor(foo, n); });
   * }
   */
  template <typename DecoratedSignature, typename Factory>
  PartialComponent<fruit::impl::RegisterFactory<DecoratedSignature, Factory>, Bindings...> registerFactory(Factory factory) &&;
  
  /**
   * Adds the bindings (and multibindings) in `component' to the current component.
   * 
   * Example usage:
   * 
   * createComponent()
   *    .install(getComponent1())
   *    .install(getComponent2())
   * 
   * As in the example, the template parameters will be inferred by the compiler, it's not necessary to specify them explicitly.
   * 
   * This supports annotated injection, just wrap the desired types (return type and/or argument types of the signature)
   * with fruit::Annotated<> if desired.
   */
  template <typename... Params>
  PartialComponent<fruit::impl::InstallComponent<Component<Params...>>, Bindings...> install(Component<Params...> component) &&;
  
private:
  template <typename... OtherBindings>
  friend class PartialComponent;
  
  template <typename... Types>
  friend class Component;
  
  fruit::impl::ComponentStorage storage;
  
  // Do not use. Use fruit::createComponent() instead.
  PartialComponent() = default;
  
  friend PartialComponent<> createComponent();
  
  // Do not use. Only use PartialComponent for temporaries, and then convert it to a Component.
  PartialComponent(const PartialComponent&) = delete;
  
  PartialComponent(fruit::impl::ComponentStorage&& storage);
  
  template <typename NewBinding>
  using OpFor = typename fruit::impl::meta::OpForComponent<Bindings...>::template AddBinding<NewBinding>;
};

} // namespace fruit

#include <fruit/impl/component.defn.h>


#endif // FRUIT_COMPONENT_H
