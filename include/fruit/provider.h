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

#ifndef FRUIT_PROVIDER_H
#define FRUIT_PROVIDER_H

// This include is not required here, but having it here shortens the include trace in error messages.
#include <fruit/impl/injection_errors.h>

#include <fruit/component.h>

namespace fruit {

/**
 * A Provider is a class that allows access to instances of the types used as parameters of the Provider template.
 * It's possible to inject a Provider<MyClass> instead of MyClass itself, and this allows lazy injection.
 * For example:
 *
 * class S {
 * private:
 *   Bar* bar = nullptr;
 *
 * public:
 *   INJECT(S(Foo* foo, Provider<Bar> barProvider)) {
 *     if (foo->needsBar()) {
 *       bar = barProvider.get();
 *     }
 *   }
 * };
 *
 * In the example above, Bar will only be created if get<Bar*> is called.
 * This can be useful if Bar is expensive to create (or some other types that need to be injected when a Bar is injected
 * are) or if there are other side effects of the Bar constructor that are undesirable when !foo->needsBar().
 * It's also possible to store the Provider object in a field, and create the Bar instance when the first method that
 * needs it is called:
 *
 * class S {
 * private:
 *   Provider<Bar> barProvider;
 *
 * public:
 *   INJECT(S(Provider<Bar> barProvider))
 *   : barProvider(barProvider) {
 *   }
 *
 *   void execute() {
 *     if (...) {
 *       Bar* bar = barProvider.get();
 *       ...
 *     }
 *   }
 * };
 *
 * As usual, Fruit ensures that (at most) one instance is ever created in a given injector; so if the Bar object was
 * already constructed, the get() will simply return it.
 *
 * Note that you can inject a Provider<Foo> whenever you could have injected a Foo.
 * It doesn't matter if Foo was bound using PartialComponent::registerProvider() or not.
 */
template <typename C>
class Provider {
private:
  using Check1 =
      typename fruit::impl::meta::CheckIfError<fruit::impl::meta::Eval<fruit::impl::meta::CheckNormalizedTypes(
          fruit::impl::meta::RemoveConstFromTypes(fruit::impl::meta::Vector<fruit::impl::meta::Type<C>>))>>::type;
  // Force instantiation of Check1.
  static_assert(true || sizeof(Check1), "");

  using Check2 =
      typename fruit::impl::meta::CheckIfError<fruit::impl::meta::Eval<fruit::impl::meta::CheckNotAnnotatedTypes(
          fruit::impl::meta::Vector<fruit::impl::meta::Type<C>>)>>::type;
  // Force instantiation of Check2.
  static_assert(true || sizeof(Check2), "");

public:
  /**
   * Returns an instance of the specified type. The following variations are allowed:
   *
   * On a Provider<Foo>, you can call:
   *
   * - provider.get<Foo>()
   * - provider.get<Foo*>()
   * - provider.get<Foo&>()
   * - provider.get<const Foo*>()
   * - provider.get<const Foo&>()
   * - provider.get<std::shared_ptr<Foo>>()
   * - provider.get<Provider<Foo>>()
   * - provider.get<Provider<const Foo>>()
   *
   * On a Provider<const Foo>, you can call:
   *
   * - provider.get<Foo>()
   * - provider.get<const Foo*>()
   * - provider.get<const Foo&>()
   * - provider.get<Provider<const Foo>>()
   *
   * The shared_ptr version is slightly slower than the ones returning a reference/pointer, use those if possible.
   *
   * Calling get<> repeatedly for the same class with the same injector will return the same instance (except for the
   * first variation above, that returns a value; in that case, another copy of the same instance will be returned).
   */
  template <typename T>
  T get();

  /**
   * This is a convenient way to call get(). E.g.:
   *
   * C& x(provider);
   *
   * is equivalent to:
   *
   * C& x = provider.get<C&>();
   */
  template <typename T>
  explicit operator T();

  /**
   * This is equivalent to get<C*>(), it's provided for convenience.
   */
  C* get();

private:
  // This is NOT owned by the provider object. It is not deleted on destruction.
  // This is never nullptr.
  fruit::impl::InjectorStorage* storage;
  fruit::impl::InjectorStorage::Graph::node_iterator itr;

  Provider(fruit::impl::InjectorStorage* storage, fruit::impl::InjectorStorage::Graph::node_iterator itr);

  friend class fruit::impl::InjectorStorage;

  template <typename T>
  friend struct fruit::impl::GetFirstStage;

  template <typename... OtherPs>
  friend class Injector;
};

} // namespace fruit

#include <fruit/impl/provider.defn.h>

#endif // FRUIT_PROVIDER_H
