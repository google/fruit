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

// The shared_ptr objects below ensure (since these tests are run under Valgrind) that deletion occurs, and only once.

struct I1 {
  std::shared_ptr<int> x = std::make_shared<int>(3);
  virtual ~I1() {}
};

struct I2 {
  std::shared_ptr<int> x = std::make_shared<int>(3);
};

struct I3 {
  std::shared_ptr<int> x = std::make_shared<int>(3);
};

struct I4 {
  std::shared_ptr<int> x = std::make_shared<int>(3);
};

struct X1 : I1 {
  using Inject = X1();
  std::shared_ptr<int> x = std::make_shared<int>(3);
};

struct X2 : I2 {
  // Taking an X1 here prevents binding compression.
  using Inject = X2(fruit::Annotated<Annotation, X1>);
  X2(X1) {}
  std::shared_ptr<int> x = std::make_shared<int>(3);
};

struct X3 : public I3 {
  std::shared_ptr<int> x = std::make_shared<int>(3);
};

struct X4 : public I4 {
  // Taking an X3 here prevents binding compression.
  X4(X3) {};
  std::shared_ptr<int> x = std::make_shared<int>(3);
};

struct X5 {
  std::shared_ptr<int> x = std::make_shared<int>(3);
};


struct X6 : public I1 {
  using Inject = X6();
  X6() = default;
  std::shared_ptr<int> x = std::make_shared<int>(3);
};

struct X7 : public I1 {
  std::shared_ptr<int> x = std::make_shared<int>(3);
};

struct X8 : public I1 {
  std::shared_ptr<int> x = std::make_shared<int>(3);
  virtual ~X8() {}
};

using I1Annot = fruit::Annotated<Annotation, I1>;
using I2Annot = fruit::Annotated<Annotation, I2>;
using I3Annot = fruit::Annotated<Annotation, I3>;
using I4Annot = fruit::Annotated<Annotation, I4>;

using X1Annot = fruit::Annotated<Annotation, X1>;
using X2Annot = fruit::Annotated<Annotation, X2>;
using X3Annot = fruit::Annotated<Annotation, X3>;
using X4Annot = fruit::Annotated<Annotation, X4>;
using X5Annot = fruit::Annotated<Annotation, X5>;
using X6Annot = fruit::Annotated<Annotation, X6>;
using X7Annot = fruit::Annotated<Annotation, X7>;
using X8Annot = fruit::Annotated<Annotation, X8>;

Component<I1Annot, I2Annot, I3Annot, I4Annot, X5Annot> getComponent() {
  static X5 x5;
  static std::unique_ptr<X7> x7(new X7());
  return fruit::createComponent()
      .bind<I1Annot, X1Annot>()
      .bind<I2Annot, X2Annot>()
      .bind<I3Annot, X3Annot>()
      .bind<I4Annot, X4Annot>()
      .registerProvider<X3Annot()>([]() { return X3(); })
      .registerProvider<X4Annot(X3Annot)>([](X3 x3) { return X4(x3); })
      .bindInstance<X5Annot>(x5)
      .addMultibinding<I1Annot, X6Annot>()
      .addInstanceMultibinding<X7Annot>(*x7)
      .addMultibindingProvider<fruit::Annotated<Annotation, X1*>()>([]() { return (X1*) new X8(); });
}

int main() {
  // Create an injector without creating any instances.
  Injector<I1Annot, I2Annot, I3Annot, I4Annot, X5Annot> injector1(getComponent());
  
  // And an injector where we do create the instances.
  Injector<I1Annot, I2Annot, I3Annot, I4Annot, X5Annot> injector2(getComponent());
  
  injector2.get<I1Annot>();
  injector2.get<I2Annot>();
  injector2.get<I3Annot>();
  injector2.get<I4Annot>();
  injector2.get<X5Annot>();
  
  injector2.getMultibindings<I1Annot>();
  
  return 0;
}
