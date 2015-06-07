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
#include <iostream>

using fruit::Component;
using fruit::Injector;

static bool c1_constructed = false;

struct I1 {};
struct C1 : public I1 {
  INJECT(C1()) {
    if (c1_constructed) {
      std::cerr << "C1 constructed twice!" << std::endl;
      exit(1);
    }
    
    c1_constructed = true;
  }
};

struct I2 {};
struct C2 : public I2 {
  INJECT(C2(I1*)) {}
};

Component<I1> getI1Component() {
  return fruit::createComponent()
      .bind<I1, C1>();
}

Component<I2> getI2Component() {
  return fruit::createComponent()
      .install(getI1Component())
      .bind<I2, C2>();
}

struct X {
  // Intentionally C1 and not I1. This prevents binding compression for the I1->C1 edge.
  INJECT(X(C1*)) {}
};

Component<X> getXComponent() {
  return fruit::createComponent();
}

int main() {
  // Here the binding C2->I1->C1 is compressed into C2->C1.
  fruit::NormalizedComponent<I2> normalizedComponent(getI2Component());
  
  // However the binding X->C1 prevents binding compression on I1->C1, the binding compression must be undone.
  Injector<I2, X> injector(normalizedComponent, getXComponent());
  
  // The check in C1's constructor ensures that only one instance of C1 is created.
  injector.get<I2*>();
  injector.get<X*>();
  
  return 0;
}
