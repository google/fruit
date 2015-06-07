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

struct Annotation1 {};
struct Annotation2 {};

struct I {
  int value = 5;
};

struct X : public I {
  X() {
    ++num_constructions;
  }
  
  static unsigned num_constructions;
};

using IAnnot = fruit::Annotated<Annotation1, I>;
using XAnnot = fruit::Annotated<Annotation2, X>;

unsigned X::num_constructions = 0;

fruit::Component<IAnnot> getComponentWithProviderByValue() {
  return fruit::createComponent()
    .registerProvider<XAnnot()>([](){return X();})
    .bind<IAnnot, XAnnot>();
}

fruit::Component<IAnnot> getComponentWithPointerProvider() {
  return fruit::createComponent()
    .registerProvider<fruit::Annotated<Annotation2, X*>()>([](){return new X();})
    .bind<IAnnot, XAnnot>();
}

int main() {
  
  Injector<IAnnot> injector1(getComponentWithProviderByValue());
  injector1.get<fruit::Annotated<Annotation1, I*>>();
  Injector<IAnnot> injector2(getComponentWithPointerProvider());
  injector2.get<fruit::Annotated<Annotation1, I*>>();
  
  Assert((injector2.get<fruit::Annotated<Annotation1, I                 >>() .value == 5));
  Assert((injector2.get<fruit::Annotated<Annotation1, I*                >>()->value == 5));
  Assert((injector2.get<fruit::Annotated<Annotation1, I&                >>() .value == 5));
  Assert((injector2.get<fruit::Annotated<Annotation1, const I           >>() .value == 5));
  Assert((injector2.get<fruit::Annotated<Annotation1, const I*          >>()->value == 5));
  Assert((injector2.get<fruit::Annotated<Annotation1, const I&          >>() .value == 5));
  Assert((injector2.get<fruit::Annotated<Annotation1, std::shared_ptr<I>>>()->value == 5));
  
  Assert(X::num_constructions == 2);
  
  return 0;
}
