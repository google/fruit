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

struct Annotation1 {};
struct Annotation2 {};

struct X {
  int n;
  
  X(int n) : n(n) {
  }
};

using XAnnot1 = fruit::Annotated<Annotation1, X>;
using XAnnot2 = fruit::Annotated<Annotation2, X>;

fruit::Component<XAnnot1, XAnnot2> getComponent() {
  fruit::Component<XAnnot1> comp = fruit::createComponent()
    .registerProvider<XAnnot1()>([](){return X(0);});
  return fruit::createComponent()
    .install(comp)
    .registerProvider<XAnnot2()>([](){return X(1);});
}

int main() {
  fruit::Injector<XAnnot1, XAnnot2> injector(getComponent());
  injector.get<XAnnot1>();
  injector.get<XAnnot2>();
  
  return 0;
}
