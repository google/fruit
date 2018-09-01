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

#ifndef FRUIT_INJECTOR_ACCESSOR_FOR_TESTS_H
#define FRUIT_INJECTOR_ACCESSOR_FOR_TESTS_H

#include <fruit/fruit.h>

namespace fruit {
namespace impl {

/**
 * A class used to access Injector's internals in Fruit's own tests. Note that this is *not* meant to be used outside
 * of Fruit.
 */
struct InjectorAccessorForTests {

  /**
   * If C was bound (directly or indirectly) in the component used to create this injector, returns a pointer to the
   * instance of C (constructing it if necessary). Otherwise returns nullptr.
   *
   * This supports annotated injection, just use Annotated<Annotation, C> instead of just C.
   * With a non-annotated parameter C, this returns a C*.
   * With an annotated parameter C=Annotated<Annotation, SomeClass>, this returns a const SomeClass*.
   *
   * Note that this doesn't trigger auto-bindings: so even if the constructor of C was visible to some get*Component
   * function (or to the place where unsafeGet is called), in order to successfully get an instance with this method
   * you need all the following to be true:
   * * C was explicitly bound in a component, or C was a dependency (direct or indirect) of a type that was explicitly
   * bound
   * * C was not bound to any interface (note however that if C was bound to I, you can do unsafeGet<I>() instead).
   *
   * Otherwise this method will return nullptr.
   */
  template <typename C, typename... Params>
  static const fruit::impl::RemoveAnnotations<C>*
  unsafeGet(fruit::Injector<Params...>& injector);
};
}
}

#include <fruit/impl/injector/injector_accessor_for_tests.defn.h>

#endif // FRUIT_INJECTOR_ACCESSOR_FOR_TESTS_H
