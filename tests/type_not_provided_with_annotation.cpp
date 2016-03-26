// expect-compile-error TypeNotProvidedError<fruit::Annotated<Annotation2,Y>>|Trying to get an instance of T, but it is not provided by this Provider/Injector.
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

#include <fruit/fruit.h>
#include "test_macros.h"

using fruit::Component;
using fruit::Injector;

struct X;
struct Y;

struct Annotation1 {};
struct Annotation2 {};

using XAnnot = fruit::Annotated<Annotation1, X>;
using YAnnot = fruit::Annotated<Annotation2, Y>;

struct X {
  using Inject = X();
};

struct Y {};

fruit::Component<XAnnot> getComponent() {
  return fruit::createComponent();
}

int main() {
  
  Injector<XAnnot> injector(getComponent());
  injector.get<YAnnot>();
  
  return 0;
}
