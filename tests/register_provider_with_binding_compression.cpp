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

struct I {
  int value = 5;
};

struct X : public I {
  X() {
    ++num_constructions;
  }
  
  static unsigned num_constructions;
};

unsigned X::num_constructions = 0;

fruit::Component<I> getComponentWithProviderByValue() {
  return fruit::createComponent()
    .registerProvider([](){return X();})
    .bind<I, X>();
}

fruit::Component<I> getComponentWithPointerProvider() {
  return fruit::createComponent()
    .registerProvider([](){return new X();})
    .bind<I, X>();
}

int main() {
  
  Injector<I> injector1(getComponentWithProviderByValue());
  injector1.get<I*>();
  Injector<I> injector2(getComponentWithPointerProvider());
  injector2.get<I*>();
  
  Assert(injector2.get<I>().value == 5);
  Assert(injector2.get<I*>()->value == 5);
  Assert(injector2.get<I&>().value == 5);
  Assert(injector2.get<const I>().value == 5);
  Assert(injector2.get<const I*>()->value == 5);
  Assert(injector2.get<const I&>().value == 5);
  Assert(injector2.get<std::shared_ptr<I>>()->value == 5);
  
  Assert(X::num_constructions == 2);
  
  return 0;
}
