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

struct X {
  INJECT(X()) = default;
  X(X&&) = default;
  X(const X&) = default;
};

struct X1 {
  X1() = default;
  X1(X1&&) = default;
  X1(const X1&) = default;
};

struct X2 {
  X2() = default;
  X2(X2&&) = default;
  X2(const X2&) = default;
};

struct Y {
  INJECT(Y()) = default;
  Y(Y&&) = default;
  Y(const Y&) = delete;
};

struct Y1 {
  Y1() = default;
  Y1(Y1&&) = default;
  Y1(const Y1&) = delete;
};

struct Y2 {
  Y2() = default;
  Y2(Y2&&) = default;
  Y2(const Y2&) = delete;
};

struct Z {
  INJECT(Z()) = default;
  Z(Z&&) = delete;
  Z(const Z&) = delete;
};

struct Z1 {
  Z1() = default;
  Z1(Z1&&) = delete;
  Z1(const Z1&) = delete;
};

struct Z2 {
  Z2() = default;
  Z2(Z2&&) = delete;
  Z2(const Z2&) = delete;
};

fruit::Component<X, X1, X2, Y, Y1, Y2> getComponent() {
  return fruit::createComponent()
    .registerProvider([](){return X1();})
    .registerProvider([](){return Y1();})
    .registerProvider([](){return new X2();})
    .registerProvider([](){return new Y2();})
    .registerProvider([](){return new Z2();});
}

template <typename T>
using Factory = std::function<T()>;

template <typename T>
using PtrFactory = std::function<std::unique_ptr<T>()>;

fruit::Component<Factory<X1>, PtrFactory<X2>, Factory<Y1>, PtrFactory<Y2>, PtrFactory<Z2>> getFactoryComponent() {
  return fruit::createComponent()
    .registerFactory<X1()>([](){return X1();})
    .registerFactory<Y1()>([](){return Y1();})
    .registerFactory<std::unique_ptr<X2>()>([](){return std::unique_ptr<X2>();})
    .registerFactory<std::unique_ptr<Y2>()>([](){return std::unique_ptr<Y2>();})
    .registerFactory<std::unique_ptr<Z2>()>([](){return std::unique_ptr<Z2>();});
}

int main() {
  Injector<X, X1, X2, Y, Y1, Y2> injector(getComponent());
  injector.get<X*>();
  injector.get<X1*>();
  injector.get<X2*>();
  injector.get<Y*>();
  injector.get<Y1*>();
  injector.get<Y2*>();
  
  Injector<Factory<X1>, PtrFactory<X2>, Factory<Y1>, PtrFactory<Y2>, PtrFactory<Z2>> injector2(getFactoryComponent());
  injector2.get<Factory<X1>>()();
  injector2.get<PtrFactory<X2>>()();
  injector2.get<Factory<Y1>>()();
  injector2.get<PtrFactory<Y2>>()();
  injector2.get<PtrFactory<Z2>>()();
  
  return 0;
}
