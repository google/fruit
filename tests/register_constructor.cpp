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

using fruit::Component;
using fruit::Injector;

struct X {
  INJECT(X()) = default;
  X(X&&) = default;
  X(const X&) = default;
};

struct Y {
  INJECT(Y()) = default;
  Y(Y&&) = default;
  Y(const Y&) = delete;
};

struct Z {
  INJECT(Z()) = default;
  Z(Z&&) = delete;
  Z(const Z&) = delete;
};

fruit::Component<X, Y, Z> getComponent() {
  return fruit::createComponent();
}

template <typename T>
using Factory = std::function<std::unique_ptr<T>()>;

fruit::Component<Factory<X>, Factory<Y> /*, Factory<Z>*/> getFactoryComponent() {
  return fruit::createComponent();
}

int main() {
  Injector<X, Y, Z> injector(getComponent());
  injector.get<X*>();
  injector.get<Y*>();
  injector.get<Z*>();
  
  Injector<Factory<X>, Factory<Y> /*, Factory<Z>*/> injector2(getFactoryComponent());
  injector2.get<Factory<X>>()();
  injector2.get<Factory<Y>>()();
  // TODO: Consider making it work for non-movable types too.
  /*
  injector2.get<Factory<Z>>()();
  */
  
  return 0;
}
