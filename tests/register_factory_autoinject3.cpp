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

struct Foo {
  Foo(X x, Y y) {
    (void)x;
    (void)y;
  }
};

using FooFactory = std::function<Foo()>;

fruit::Component<FooFactory> getComponent() {
  static X x = X();
  static Y y = Y();
  return fruit::createComponent()
      .bindInstance(x)
      .bindInstance(y)
      .registerFactory<Foo(X, Y)>(
          [](X x, Y y) {
            return Foo(x, y);
          });
}


int main() {
  fruit::Injector<FooFactory> injector(getComponent());
  FooFactory fooFactory(injector);
  Foo foo = fooFactory();
  (void)foo;
  
  return 0;
}
