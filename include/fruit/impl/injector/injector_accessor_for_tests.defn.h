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

#ifndef FRUIT_INJECTOR_ACCESSOR_FOR_TESTS_DEFN_H
#define FRUIT_INJECTOR_ACCESSOR_FOR_TESTS_DEFN_H

#include <fruit/impl/injector/injector_accessor_for_tests.h>

namespace fruit {
namespace impl {

template <typename AnnotatedC, typename... Params>
const fruit::impl::RemoveAnnotations<AnnotatedC>*
InjectorAccessorForTests::unsafeGet(fruit::Injector<Params...>& injector) {
  using Op = fruit::impl::meta::Eval<fruit::impl::meta::CheckNormalizedTypes(
      fruit::impl::meta::Vector<fruit::impl::meta::Type<AnnotatedC>>)>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();
  return injector.storage->template unsafeGet<AnnotatedC>();
}
}
}

#endif // FRUIT_INJECTOR_ACCESSOR_FOR_TESTS_DEFN_H
