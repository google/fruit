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

struct X {
  X() {
    ++num_constructions;
  }
  
  static unsigned num_constructions;
  
  int value = 5;
};

unsigned X::num_constructions = 0;

fruit::Component<X> getComponentWithProviderByValue() {
  return fruit::createComponent()
    .registerProvider([](){return X();});
}

fruit::Component<X> getComponentWithPointerProvider() {
  return fruit::createComponent()
    .registerProvider([](){return new X();});
}

int main() {
  
  Injector<X> injector1(getComponentWithProviderByValue());
  injector1.get<X*>();
  Injector<X> injector2(getComponentWithPointerProvider());
  injector2.get<X*>();
  
  Assert(injector2.get<X>().value == 5);
  Assert(injector2.get<X*>()->value == 5);
  Assert(injector2.get<X&>().value == 5);
  Assert(injector2.get<const X>().value == 5);
  Assert(injector2.get<const X*>()->value == 5);
  Assert(injector2.get<const X&>().value == 5);
  Assert(injector2.get<std::shared_ptr<X>>()->value == 5);
  
  Assert(X::num_constructions == 2);
  
  return 0;
}
