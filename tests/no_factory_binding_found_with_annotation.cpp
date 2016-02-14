// expect-compile-error NoBindingFoundError<fruit::Annotated<Annotation,std::\(__1::\)\?function<std::\(__1::\)\?unique_ptr<X\(,std::\(__1::\)\?default_delete<X>\)\?>()>>|No explicit binding nor C::Inject definition was found for T.
// The __1:: prefix appears when using libc++

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

struct Annotation {};

struct X {
};

using XAnnot = fruit::Annotated<Annotation, X>;

fruit::Component<fruit::Annotated<Annotation, std::function<std::unique_ptr<X>()>>> getComponent() {
  return fruit::createComponent();
}
