// expect-runtime-error error: the type X was provided more than once, with different bindings.
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
  INJECT(X()) = default;
};

fruit::Component<X> getComponent() {
  return fruit::createComponent();
}

fruit::Component<X> getComponent2() {
  static X x;
  return fruit::createComponent()
      .bindInstance(x);
}

int main() {
  
  fruit::NormalizedComponent<> normalizedComponent(getComponent());
  fruit::Injector<X> injector(normalizedComponent, getComponent2());
  
  injector.get<X>();
  
  return 0;
}
