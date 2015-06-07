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

struct X {
};

struct Y {
  INJECT(Y(X)) {};
};

fruit::Component<fruit::Required<X>, Y> getComponent() {
  return fruit::createComponent();
}

fruit::Component<X> getXComponent(X& x) {
  return fruit::createComponent()
    .bindInstance(x);
}

int main() {
  fruit::NormalizedComponent<fruit::Required<X>, Y> normalizedComponent(getComponent());
  
  X x{};
  
  fruit::Injector<X> injector(normalizedComponent, fruit::Component<X>(fruit::createComponent().bindInstance(x)));
  injector.get<X*>();
  
  fruit::Injector<Y> injector2(normalizedComponent, getXComponent(x));
  injector2.get<Y*>();
  
  fruit::Injector<Y> injector3(normalizedComponent, getXComponent(x));
  injector3.get<Y*>();
  
  fruit::Injector<X> injector4(normalizedComponent, fruit::Component<X>(fruit::createComponent().bindInstance(x)));
  injector4.get<X*>();
  
  return 0;
}
