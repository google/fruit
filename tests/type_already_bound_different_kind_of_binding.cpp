// expect-compile-error TypeAlreadyBoundError<X>|Trying to bind C but it is already bound.
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
using fruit::createComponent;

struct X {
};

struct Y : public X {
};

Component<Y> getComponent() {
  return createComponent()
    .registerConstructor<X()>()
    .registerConstructor<Y()>()
    .bind<X, Y>();
}
