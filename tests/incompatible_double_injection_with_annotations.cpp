// expect-runtime-error Fatal injection error: the type fruit::Annotated<Annotation, X> was provided more than once, with different bindings.
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

struct Annotation {};

struct X {
  int n;
  
  X(int n) : n(n) {
  }
};

using XAnnot = fruit::Annotated<Annotation, X>;

fruit::Component<> getComponent() {
  fruit::Component<> comp = fruit::createComponent()
    .registerProvider<XAnnot()>([](){return X(0);});
  return fruit::createComponent()
    .install(comp)
    .registerProvider<XAnnot()>([](){return X(1);});
}

int main() {
  fruit::Injector<> injector(getComponent());
  
  return 0;
}
