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
using fruit::Assisted;
using fruit::createComponent;

struct X {};
struct Y {};
struct Z {};

struct Foo {
  Foo(X, Y, int, float, Z) {
  }
};

using FooFactory = std::function<Foo(int, float)>;

fruit::Component<FooFactory> getComponent() {
  static X x = X();
  static Y y = Y();
  static Z z = Z();
  return fruit::createComponent()
      .bindInstance(x)
      .bindInstance(y)
      .bindInstance(z)
      .registerFactory<Foo(X, Y, fruit::Assisted<int>, fruit::Assisted<float>, Z)>(
          [](X x, Y y, int n, float a, Z z) {
            return Foo(x, y, n, a, z);
          });
}


int main() {
  fruit::Injector<FooFactory> injector(getComponent());
  FooFactory fooFactory(injector);
  Foo foo = fooFactory(1, 3.4);
  (void)foo;
  
  return 0;
}
