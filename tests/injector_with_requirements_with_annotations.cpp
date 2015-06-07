// expect-compile-error InjectorWithRequirementsError<fruit::Annotated<Annotation,Y>>|Injectors can.t have requirements.
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

struct Y {
};

struct X {
  using Inject = fruit::Annotated<Annotation, X>(fruit::Annotated<Annotation, Y>);
  X(Y) {
  }
};

using XAnnot = fruit::Annotated<Annotation, X>;
using YAnnot = fruit::Annotated<Annotation, Y>;

fruit::Component<fruit::Required<YAnnot>, XAnnot> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::Injector<fruit::Required<YAnnot>, XAnnot> injector(getComponent());
  return 0;
}
