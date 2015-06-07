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
using fruit::createComponent;

struct Annotation1 {};
struct Annotation2 {};

struct X {
};

using XAnnot1 = fruit::Annotated<Annotation1, X>;
using XAnnot2 = fruit::Annotated<Annotation2, X>;

void f(Component<fruit::Required<XAnnot1>, XAnnot2>);

int main() {
  return 0;
}
