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

struct Annotation {};

struct X {
  using Inject = X();
  X() = default;
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
  using Inject = Y();
  Y() = default;
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
  using Inject = Z();
  Z() = default;
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

using XAnnot = fruit::Annotated<Annotation, X>;
using X1Annot = fruit::Annotated<Annotation, X1>;
using X2Annot = fruit::Annotated<Annotation, X2>;
using YAnnot = fruit::Annotated<Annotation, Y>;
using Y1Annot = fruit::Annotated<Annotation, Y1>;
using Y2Annot = fruit::Annotated<Annotation, Y2>;
using ZAnnot = fruit::Annotated<Annotation, Z>;
using Z1Annot = fruit::Annotated<Annotation, Z1>;
using Z2Annot = fruit::Annotated<Annotation, Z2>;

fruit::Component<XAnnot, X1Annot, X2Annot, YAnnot, Y1Annot, Y2Annot> getComponent() {
  return fruit::createComponent()
    .registerProvider<X1Annot()>([](){return X1();})
    .registerProvider<Y1Annot()>([](){return Y1();})
    .registerProvider<fruit::Annotated<Annotation, X2*>()>([](){return new X2();})
    .registerProvider<fruit::Annotated<Annotation, Y2*>()>([](){return new Y2();})
    .registerProvider<fruit::Annotated<Annotation, Z2*>()>([](){return new Z2();});
}

template <typename T>
using FactoryAnnot = fruit::Annotated<Annotation, std::function<T()>>;

template <typename T>
using PtrFactoryAnnot = fruit::Annotated<Annotation, std::function<std::unique_ptr<T>()>>;

fruit::Component<FactoryAnnot<X1>, PtrFactoryAnnot<X2>, FactoryAnnot<Y1>, PtrFactoryAnnot<Y2>, PtrFactoryAnnot<Z2>> getFactoryComponent() {
  return fruit::createComponent()
    .registerFactory<X1Annot()>([](){return X1();})
    .registerFactory<Y1Annot()>([](){return Y1();})
    .registerFactory<fruit::Annotated<Annotation, std::unique_ptr<X2>>()>([](){return std::unique_ptr<X2>();})
    .registerFactory<fruit::Annotated<Annotation, std::unique_ptr<Y2>>()>([](){return std::unique_ptr<Y2>();})
    .registerFactory<fruit::Annotated<Annotation, std::unique_ptr<Z2>>()>([](){return std::unique_ptr<Z2>();});
}

int main() {
  Injector<XAnnot, X1Annot, X2Annot, YAnnot, Y1Annot, Y2Annot> injector(getComponent());
  injector.get<fruit::Annotated<Annotation, X* >>();
  injector.get<fruit::Annotated<Annotation, X1*>>();
  injector.get<fruit::Annotated<Annotation, X2*>>();
  injector.get<fruit::Annotated<Annotation, Y* >>();
  injector.get<fruit::Annotated<Annotation, Y1*>>();
  injector.get<fruit::Annotated<Annotation, Y2*>>();
  
  Injector<FactoryAnnot<X1>, PtrFactoryAnnot<X2>, FactoryAnnot<Y1>, PtrFactoryAnnot<Y2>, PtrFactoryAnnot<Z2>> injector2(getFactoryComponent());
  injector2.get<FactoryAnnot<X1>>()();
  injector2.get<PtrFactoryAnnot<X2>>()();
  injector2.get<FactoryAnnot<Y1>>()();
  injector2.get<PtrFactoryAnnot<Y2>>()();
  injector2.get<PtrFactoryAnnot<Z2>>()();
  
  return 0;
}
