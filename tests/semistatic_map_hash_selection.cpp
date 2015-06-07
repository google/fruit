// expect-success
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

struct X1 {};
struct X2 {};
struct X3 {};
struct X4 {};
struct X5 {};
struct X6 {};
struct X7 {};

fruit::Component<> getComponent() {
  return fruit::createComponent()
    .registerConstructor<X1()>()
    .registerConstructor<X2()>()
    .registerConstructor<X3()>()
    .registerConstructor<X4()>()
    .registerConstructor<X5()>()
    .registerConstructor<X6()>()
    .registerConstructor<X7()>();
}

int main() {
  // The component normalization generates a random hash. By looping 50 times it's very likely that we'll get at least one hash
  // with too many collisions (and we'll generate another).
  for (int i = 0; i < 50; i++) {
    fruit::NormalizedComponent<> normalizedComponent(getComponent());
    (void) normalizedComponent;
  }
  
  return 0;
}
