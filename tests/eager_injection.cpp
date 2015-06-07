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
  INJECT(X()) {
    Assert(!constructed);
    constructed = true;
  }
  
  static bool constructed;
};

bool X::constructed = false;

struct Y {
  Y() {
    Assert(!constructed);
    constructed = true;
  }
  
  static bool constructed;
};

bool Y::constructed = false;

struct Z {
  Z() {
    Assert(!constructed);
    constructed = true;
  }
  
  static bool constructed;
};

bool Z::constructed = false;


fruit::Component<X> getComponent() {
  return fruit::createComponent()
    .addMultibindingProvider([](){return new Y();})
    .registerConstructor<Z()>();
}

int main() {
  
  Injector<X> injector(getComponent());
  
  Assert(!X::constructed);
  Assert(!Y::constructed);
  Assert(!Z::constructed);
  
  injector.eagerlyInjectAll();
  
  Assert(X::constructed);
  Assert(Y::constructed);
  // Z still not constructed, it's not reachable from Injector<X>.
  Assert(!Z::constructed);
  
  return 0;
}
