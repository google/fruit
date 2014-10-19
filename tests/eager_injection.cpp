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

using fruit::Component;
using fruit::Injector;

struct X {
  INJECT(X()) {
    assert(!constructed);
    constructed = true;
  }
  
  static bool constructed;
};

bool X::constructed = false;

struct Y {
  Y() {
    assert(!constructed);
    constructed = true;
  }
  
  static bool constructed;
};

bool Y::constructed = false;

struct Z {
  Z() {
    assert(!constructed);
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
  
  assert(!X::constructed);
  assert(!Y::constructed);
  assert(!Z::constructed);
  
  injector.eagerlyInjectAll();
  
  assert(X::constructed);
  assert(Y::constructed);
  // Z still not constructed, it's not reachable from Injector<X>.
  assert(!Z::constructed);
  
  return 0;
}
