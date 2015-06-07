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

struct Annotation {};

struct X {
};

using XAnnot = fruit::Annotated<Annotation, X>;

struct Y {
  INJECT(Y(ANNOTATED(Annotation, X))) {};
};

fruit::Component<fruit::Required<XAnnot>, Y> getComponent() {
  return fruit::createComponent();
}

fruit::Component<XAnnot> getXComponent(X& x) {
  return fruit::createComponent()
    .bindInstance<XAnnot>(x);
}

int main() {
  fruit::NormalizedComponent<fruit::Required<XAnnot>, Y> normalizedComponent(getComponent());
  
  X x{};
  
  fruit::Injector<XAnnot> injector(normalizedComponent, fruit::Component<XAnnot>(fruit::createComponent().bindInstance<XAnnot>(x)));
  injector.get<fruit::Annotated<Annotation, X*>>();
  
  fruit::Injector<Y> injector2(normalizedComponent, getXComponent(x));
  injector2.get<Y*>();
  
  fruit::Injector<Y> injector3(normalizedComponent, getXComponent(x));
  injector3.get<Y*>();
  
  fruit::Injector<XAnnot> injector4(normalizedComponent, fruit::Component<XAnnot>(fruit::createComponent().bindInstance<XAnnot>(x)));
  injector4.get<fruit::Annotated<Annotation, X*>>();
  
  return 0;
}
