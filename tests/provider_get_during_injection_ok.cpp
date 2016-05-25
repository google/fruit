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

using fruit::Injector;
using fruit::Component;
using fruit::Provider;

struct X {
  INJECT(X()) = default;
  void foo() {
  }
};

struct Y {
  X x;
  INJECT(Y(Provider<X> xProvider))
    : x(xProvider.get<X>()) {
  }
  
  void foo() {
    x.foo();
  }
};

struct Z {
  Y y;
  INJECT(Z(Provider<Y> yProvider))
  : y(yProvider.get<Y>()) {
    
  }
  
  void foo() {
    y.foo();
  }
};

Component<Z> getZComponent() {
  return fruit::createComponent();
}

int main() {
  Injector<Z> injector(getZComponent());
  Provider<Z> provider(injector);
  // During provider.get<Z>(), yProvider.get() is called, and during that xProvider.get()
  // is called.
  Z z = provider.get<Z>();
  z.foo();
  return 0;
}
